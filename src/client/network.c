#include <stdint.h>
#include <stdbool.h>
#include <string.h>
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

#include "../common/log.h"
#include "../common/packets.h"
#include "event.h"
#include "network.h"

int initialize_client_network_context(client_network_context* ctx, char* server_ipv4, char* server_port) {
	// Get address info for server domain/ip and port
	int status = 0;
	struct addrinfo sock_stream_hints;

	// Get addres info of the server
	memset(&sock_stream_hints, 0, sizeof(sock_stream_hints));
	sock_stream_hints.ai_family = AF_INET;
	sock_stream_hints.ai_socktype = SOCK_STREAM;
	if((status = getaddrinfo(server_ipv4, server_port, &sock_stream_hints, &ctx->tcp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		indent_print_log(1, "server address: '%s' of length %d", server_ipv4, strlen(server_ipv4));
		indent_print_log(1, "server port: '%s' of length %d", server_port, strlen(server_port));
		return -1;
	}

	// Create a TCP socket to connect to the server with
	ctx->tcp_fd = socket(ctx->tcp_info->ai_family, ctx->tcp_info->ai_socktype, ctx->tcp_info->ai_protocol); // Create server socket stream
	if(ctx->tcp_fd == -1) {
		print_log("Unable to create socket stream for server! Errno: %d", errno);
		return -1;
	}
	ctx->tcp_pollfd = &ctx->pollfds[0];
	ctx->tcp_pollfd->fd = ctx->tcp_fd;
	ctx->tcp_pollfd->events = POLLIN;

	// Make connection before setting the socket file descriptor to be non-blocking
	if(connect(ctx->tcp_fd, ctx->tcp_info->ai_addr, ctx->tcp_info->ai_addrlen) == -1) {
		print_log("Connection to server has failed: %d\n", errno);
		return -1;
	}
	print_log("Connection to server established");
	set_nonblocking(ctx->tcp_fd);

	// Repeat the process for the UDP DATAGRAM_SOCKET
	struct addrinfo sock_dgram_hints;
	memset(&sock_dgram_hints, 0, sizeof(sock_dgram_hints));
	sock_dgram_hints.ai_family = AF_INET;
	sock_dgram_hints.ai_socktype = SOCK_DGRAM;
	if((status = getaddrinfo(server_ipv4, server_port, &sock_dgram_hints, &ctx->udp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		return -1;
	}

	ctx->udp_fd = socket(ctx->udp_info->ai_family, ctx->udp_info->ai_socktype, ctx->udp_info->ai_protocol); // Create server socket datagram
	if(ctx->udp_fd == -1) {
		print_log("Unable to create socket datagram for server! Errno: %d", errno);
		return -1;
	}
	ctx->udp_pollfd = &ctx->pollfds[1];
	ctx->udp_pollfd->fd = ctx->udp_fd;
	ctx->udp_pollfd->events = POLLIN;
	set_nonblocking(ctx->udp_fd);

	ctx->id = -1; // Should get a welcome packet for client id

	return 0;
}

void close_client_network_context(client_network_context* ctx) {
	server_packet disconnect_packet;
	disconnect_packet.type = SERVER_DISCONNECT_PAYLOAD;
	disconnect_packet.sender_id = ctx->id;

	send_packet(ctx, &disconnect_packet);

	if(ctx->tcp_fd != -1) close(ctx->tcp_fd);
	if(ctx->udp_fd != -1) close(ctx->udp_fd);

	freeaddrinfo(ctx->tcp_info);
	freeaddrinfo(ctx->udp_info);
}

int set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		return -1;
	}
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return 0;
}

int client_network_update(client_network_context* ctx) {
	if(ctx == NULL) {
		return -1;
	}

	poll_client_events(ctx);

	return handle_packet_receive(ctx);
}

int poll_client_events(client_network_context* ctx) {
	 // Update poll fds to contain events that occured
	if(poll(ctx->pollfds, 2, 0) == -1) {
		print_log("Poll failed! Errno: %d", errno);
		return -1;
	}
	return 0;
}

int send_packet(client_network_context* ctx, server_packet* packet) {
	packet->sender_id = ctx->id;

	ssize_t bytes = send(ctx->tcp_fd, (void*) packet, sizeof(server_packet), 0);

	if(bytes < 0) {
		if((long unsigned int)bytes < sizeof(client_packet)) {
			print_log("Unable to send entire packet. Sent %d/%d bytes", bytes, sizeof(client_packet));
			return -1;
		}
		print_log("Unable to send client packet");
		return -1;
	}

	return 0;
}
