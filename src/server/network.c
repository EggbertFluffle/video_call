#include <stdint.h>

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
	ctx->pollfds[0].fd = 0;
	ctx->pollfd_count++;

	// Get address info for server domain/ip and port
	int status = 0;
	struct addrinfo tcp_hints;

	memset(&tcp_hints, 0, sizeof(tcp_hints));
	tcp_hints.ai_family = AF_INET;
	tcp_hints.ai_socktype = SOCK_STREAM;
	if((status = getaddrinfo(server_ipv4, server_port, &tcp_hints, &ctx->tcp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		return -1;
	}

	// Create a TCP socket to listen for connections with
	ctx->tcp_fd = socket(ctx->tcp_info->ai_family, ctx->tcp_info->ai_socktype, ctx->tcp_info->ai_protocol); // Create server socket stream
	if(ctx->tcp_fd == -1) {
		print_log("Unable to create socket stream for server! Errno: %d", errno);
		return -1;
	}

	// TCP socket is the second pollable fd
	ctx->pollfds[1].fd = ctx->tcp_fd;
	ctx->pollfd_count++;

	// Bind the TCP to the desired port
	if(bind(ctx->tcp_fd, ctx->tcp_info->ai_addr, ctx->tcp_info->ai_addrlen) == -1) {
		print_log("Unable to bind to port %s", server_port);
		return -1;
	}

	// Repeat the process for the UDP DATAGRAM_SOCKET
	struct addrinfo udp_hints;
	memset(&udp_hints, 0, sizeof(udp_hints));
	udp_hints.ai_family = AF_INET;
	udp_hints.ai_socktype = SOCK_DGRAM;
	if((status = getaddrinfo(server_ipv4, server_port, &udp_hints, &ctx->udp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		return -1;
	}

	ctx->udp_fd= socket(ctx->udp_info->ai_family, ctx->udp_info->ai_socktype, ctx->udp_info->ai_protocol);
	if(ctx->udp_fd== -1) {
		print_log("Unable to create socket datagram for server! Errno: %d", errno);
		return -1;
	}
	ctx->pollfds[2].fd = ctx->udp_fd;
	ctx->pollfd_count++;

	for(int fd = 0; fd < ctx->pollfd_count; fd++) {
		ctx->pollfds[2].events = POLLIN;
		int opts = fcntl(ctx->pollfds[fd].fd, F_GETFL, 0);
		fcntl(ctx->pollfds[fd].fd, F_SETFL, opts | O_NONBLOCK);
	}

	ctx->client_count = 0;
	for(int c = 0; c < MAX_CLIENTS; c++) {
		ctx->clients[c].fd = -1;
	}

	return 0;
}

void close_server_network_context(server_network_context* ctx) {
	close(ctx->pollfds[0].fd); // IDK if we actually have to close stdin
	if(ctx->tcp_fd != -1) close(ctx->tcp_fd);
	if(ctx->udp_fd != -1) close(ctx->udp_fd);

	freeaddrinfo(ctx->tcp_info);
	freeaddrinfo(ctx->udp_info);
}
