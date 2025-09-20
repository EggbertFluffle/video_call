#include <stdint.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>

#include <poll.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>

#include "network.h"
#include "../common/log.h"

int initialize_server_network_context(server_network_context* ctx, char* server_ipv4, char* server_port) {
	// Stdin is the first pollable fd
	ctx->stdin_pollfd = &ctx->pollfds[0];
	ctx->stdin_pollfd->fd = STDIN_FILENO;
	ctx->pollfd_count++;

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

	// TCP socket is the second pollable fd
	ctx->tcp_pollfd = &ctx->pollfds[1];
	ctx->tcp_pollfd->fd = ctx->tcp_fd;
	ctx->pollfd_count++;

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
	ctx->pollfd_count++;

	for(int fd = 0; fd < ctx->pollfd_count; fd++) {
		ctx->pollfds[fd].events = POLLIN;
		int opts = fcntl(ctx->pollfds[fd].fd, F_GETFL, 0);
		fcntl(ctx->pollfds[fd].fd, F_SETFL, opts | O_NONBLOCK);
	}

	ctx->client_count = 0;
	for(int c = 0; c < MAX_CLIENTS - 1; c++) {
		ctx->clients[c].pollfd = &ctx->pollfds[c + 3];
		ctx->clients[c].pollfd->fd = -1;
	}

	if(listen(ctx->tcp_fd, 20) != 0) {
		print_log("Error listening for connections! Errno: %d", errno);
		close_server_network_context(ctx);
		return -1;
	}
	print_log("Listening...");

	return 0;
}

void close_server_network_context(server_network_context* ctx) {
	for(int fd = 0; fd < ctx->pollfd_count; fd++) {
		close(ctx->pollfds[fd].fd);
	}

	freeaddrinfo(ctx->tcp_info);
	freeaddrinfo(ctx->udp_info);
}

int accept_client(server_network_context* ctx) {
	if(ctx->tcp_pollfd->revents & POLLIN) {
		int client_fd;
		if((client_fd = accept(ctx->tcp_fd, NULL, NULL)) == -1) {
			print_log("Socket accepting failed");
			return -1;
		}

		if(ctx->pollfd_count == MAX_CLIENTS) {
			print_log("Max number of clients reached, connection rejected");
		}

		struct pollfd* new_pollfd = &ctx->pollfds[ctx->pollfd_count];
		new_pollfd->fd = client_fd;
		new_pollfd->events = new_pollfd->events | POLLIN | POLLHUP;
		int opts = fcntl(new_pollfd->fd, F_GETFD, 0);
		fcntl(new_pollfd->fd, F_SETFD, opts | O_NONBLOCK);
		ctx->pollfd_count++;

		client_connection* new_client_connection = &ctx->clients[ctx->client_count];
		new_client_connection->pollfd = new_pollfd;
		new_client_connection->id = ctx->client_count;
		ctx->client_count++;

		print_log("New client id %d accepted", new_client_connection->id);
	}

	return 0;
}

int remove_client(server_network_context* ctx, int id) {
	client_connection* client = NULL;

	for(int c = 0; c < MAX_CLIENTS; c++) {
		if(ctx->clients[c].id == id) {
			client = &ctx->clients[c];
		}
	}

	if(client == NULL) {
		print_log("Client of id %d to be removed cannot be found", id);
		return -1;
	}

	close(client->pollfd->fd);
	client->pollfd->fd = -1;
	client->pollfd->events = client->pollfd->events & ~(POLLHUP | POLLIN);

	print_log("Removed client id %d", id);

	return 0;
}

int poll_server_events(server_network_context* ctx) {
	if(poll(ctx->pollfds, ctx->pollfd_count, -1) == -1) {
		print_log("Poll failed! Errno: %d", errno);
		return -1;
	}
	return 0;
}
