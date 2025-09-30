#include <asm-generic/errno-base.h>
#include <stdint.h>

#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>

#include <time.h>
#include <poll.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>

#include "network.h"
#include "../common/log.h"
#include "../common/packets.h"

int initialize_server_network_context(server_network_context* ctx, char* server_ipv4, char* server_port) {
	srand(time(NULL));

	// Stdin is the first pollable fd
	ctx->stdin_pollfd = &ctx->pollfds[0];
	ctx->stdin_pollfd->fd = STDIN_FILENO;

	// Get address info for server domain/ip and port
	int status = 0;
	struct addrinfo tcp_hints;

	memset(&tcp_hints, 0, sizeof(tcp_hints));
	tcp_hints.ai_family = AF_INET;
	tcp_hints.ai_socktype = SOCK_STREAM;
	if((status = getaddrinfo(server_ipv4, server_port, &tcp_hints, &ctx->tcp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		close_server_network_context(ctx);
		return -1;
	}

	// Create a TCP socket to listen for connections with
	ctx->tcp_fd = socket(ctx->tcp_info->ai_family, ctx->tcp_info->ai_socktype, ctx->tcp_info->ai_protocol); // Create server socket stream
	if(ctx->tcp_fd == -1) {
		print_log("Unable to create socket stream for server! Errno: %d", errno);
		close_server_network_context(ctx);
		return -1;
	}

	// TCP socket for connection to the server is the second pollable fd
	ctx->tcp_pollfd = &ctx->pollfds[1];
	ctx->tcp_pollfd->fd = ctx->tcp_fd;

	// Bind the TCP to the desired port
	if(bind(ctx->tcp_fd, ctx->tcp_info->ai_addr, ctx->tcp_info->ai_addrlen) == -1) {
		print_log("Unable to bind to port %s", server_port);
		close_server_network_context(ctx);
		return -1;
	}

	// Repeat the process for the UDP DATAGRAM_SOCKET
	struct addrinfo udp_hints;
	memset(&udp_hints, 0, sizeof(udp_hints));
	udp_hints.ai_family = AF_INET;
	udp_hints.ai_socktype = SOCK_DGRAM;
	if((status = getaddrinfo(server_ipv4, server_port, &udp_hints, &ctx->udp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		close_server_network_context(ctx);
		return -1;
	}

	ctx->udp_fd= socket(ctx->udp_info->ai_family, ctx->udp_info->ai_socktype, ctx->udp_info->ai_protocol);
	if(ctx->udp_fd== -1) {
		print_log("Unable to create socket datagram for server! Errno: %d", errno);
		close_server_network_context(ctx);
		return -1;
	}
	ctx->udp_pollfd = &ctx->pollfds[2];
	ctx->udp_pollfd->fd = ctx->udp_fd;

	// Make all pollfds care about POLLIN events and set them to nonblocking
	for(int fd = 0; fd < MAX_POLLFDS; fd++) {
		ctx->pollfds[fd].events = POLLIN;
		set_nonblocking(ctx->pollfds[fd].events);
	}

	// Initialize all clients with their corresponding pollfds
	// Client is invalid if id is -1
	ctx->client_count = 0;
	for(int c = 0; c < MAX_CLIENTS; c++) {
		ctx->clients[c].pollfd = &ctx->pollfds[c + RESERVED_POLLFDS];
		ctx->clients[c].id = -1;
		ctx->clients[c].pollfd->fd = -1;
	}

	if(listen(ctx->tcp_fd, LISTEN_QUEUE_LENGTH) != 0) {
		print_log("Error listening for connections! Errno: %d", errno);
		close_server_network_context(ctx);
		return -1;
	}
	print_log("Listening...");

	return 0;
}

int set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}

void close_server_network_context(server_network_context* ctx) {
	for(int fd = 0; fd < MAX_POLLFDS; fd++) {
		close(ctx->pollfds[fd].fd);
	}

	freeaddrinfo(ctx->tcp_info);
	freeaddrinfo(ctx->udp_info);
}

int poll_server_events(server_network_context* ctx) {
	if(poll(ctx->pollfds, MAX_POLLFDS, 0) == -1) {
		print_log("Poll failed! Errno: %d", errno);
		return -1;
	}
	return 0;
}

int accept_client(server_network_context* ctx) {
	if (ctx->tcp_pollfd->revents & (POLLNVAL | POLLERR)) {
		print_log("FATAL ERROR: TCP listening socket has became invalid, or errored, ending connection");
		indent_print_log(1, "Events result:");
		indent_print_log(2, "POLLNVAL: %s", ctx->tcp_pollfd->revents & POLLNVAL ? "true" : "false");
		indent_print_log(2, "POLLERR: %s", ctx->tcp_pollfd->revents & POLLERR ? "true" : "false");

		exit(-1);
	}

	if(!(ctx->tcp_pollfd->revents & POLLIN)) { 
		return 0;
	}

	int client_fd = accept(ctx->tcp_fd, NULL, NULL);

	if(client_fd < 0) {
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			print_log("No connections to be accepted");
			return 0;
		}

		// Accept was interrupted, try again
		if(errno == EINTR) {
			return accept_client(ctx);
		}

		print_log("Failed to accept client connection");
		return -1;
	}

	// Check for max clients
	if(ctx->client_count == MAX_CLIENTS) {
		print_log("Max number of clients reached, connection rejected");
		return -1;
	}

	client_connection* new_client_connection = NULL;

	// Search for next available client connection slot
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(ctx->clients[c].id == -1) {
			new_client_connection = &ctx->clients[c];
			print_log("New client connection entering slot %d", c);
			break;
		}
	}

	if(new_client_connection == NULL) {
		print_log("No client slots available, this isn't supposed to happen");
		return -1;
	}

	// Generate a random id until unique
	// TODO: Maybe constrain the id so that it is in a range, 
	// or has a fixes number of signifigant digits
	// It is possible that two clients receive the same random id
	// TODO: Fix the impossibility I guess
	do {
		new_client_connection->id = rand();
	} while(new_client_connection->id == 0);
	ctx->client_count++;

	struct pollfd* new_pollfd = new_client_connection->pollfd;
	// Set proper poll events
	new_pollfd->fd = client_fd;
	new_pollfd->events = new_pollfd->events | POLLIN | POLLHUP;
	set_nonblocking(new_pollfd->fd);

	print_log("New client id %d accepted", new_client_connection->id);

	client_packet welcome_packet;
	welcome_packet.type = CLIENT_WELCOME_PAYLOAD;
	welcome_packet.payload.welcome.client_id = new_client_connection->id;

	if(send_packet(ctx, new_client_connection->id, &welcome_packet) == -1) {
		print_log("Failed to send welcome packet to client");
		remove_client(ctx, new_client_connection->id);
		return -1;
	}

	return 0;
}

// TODO: Compress the list after removal
int remove_client(server_network_context* ctx, int id) {
	client_connection* client = get_client(ctx, id);;

	if(client == NULL) {
		print_log("Client of id %d to be removed cannot be found", id);
		return -1;
	}

	close(client->pollfd->fd);
	client->id = -1;
	client->pollfd->fd = -1;
	client->pollfd->events = 0;

	print_log("Removed client id %d", id);

	return 0;
}

client_connection* get_client(server_network_context* ctx, int id) {
	client_connection* client = NULL;
	
	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(ctx->clients[c].id == id) {
			client = &ctx->clients[c];
			break;
		}
	}

	return client;
}

int send_packet(server_network_context* ctx, int client_id, client_packet* packet) {
	if(client_id > 0) {
		client_connection* client = get_client(ctx, client_id);
		if(client == NULL) {
			print_log("Client with id %d does not exist in connected clients lists", client_id);
			return -1;
		}

		int client_fd = client->pollfd->fd;
		print_log("Sending a packet to client: %d", client->id);

		ssize_t bytes = send(client_fd, (void*)packet, sizeof(client_packet), 0);
		if(bytes < 0) {
			if((long unsigned int)bytes < sizeof(client_packet)) {
				print_log("Unable to send entire packet. Sent %d/%d bytes", bytes, sizeof(client_packet));
				return -1;
			}
			print_log("Unable to send client packet");
			return -1;
		}
	} else {
		for(int c = 0; c < ctx->client_count; c++) {
			client_connection* client = &ctx->clients[c];
			if(client == NULL) continue;

			if(client_id < 0 && client->id == -client_id) continue;

			if(send_packet(ctx, client->id, packet) == -1) {
				print_log("Unable to send packet to client %d in slot %d ", client->id, c);
			}
		}
	}

	return 0;
}
