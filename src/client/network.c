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

#include "../common/log.h"
#include "network.h"
#include "../common/utils.h"

int initialize_client_network_context(client_network_context* ctx, char* server_ipv4, char* server_port) {
	ctx->pollfds[0].fd = 0; // Add stdin to the poll list
	ctx->pollfds[0].events = POLLIN;
	int opts = fcntl(ctx->pollfds[0].fd, F_GETFL, 0); // Get the current socket options
	fcntl(ctx->pollfds[0].fd, F_SETFL, opts | O_NONBLOCK); // Stdin should be non-blocking

	// Get address info for server domain/ip and port
	int status = 0;
	struct addrinfo sock_stream_hints;

	// Get addres info of the server
	memset(&sock_stream_hints, 0, sizeof(sock_stream_hints));
	sock_stream_hints.ai_family = AF_INET;
	sock_stream_hints.ai_socktype = SOCK_STREAM;
	if((status = getaddrinfo(server_ipv4, server_port, &sock_stream_hints, &ctx->tcp_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		return -1;
	}

	// Create a TCP socket to connect to the server with
	ctx->tcp_fd = socket(ctx->tcp_info->ai_family, ctx->tcp_info->ai_socktype, ctx->tcp_info->ai_protocol); // Create server socket stream
	if(ctx->tcp_fd == -1) {
		print_log("Unable to create socket stream for server! Errno: %d", errno);
		return -1;
	}
	ctx->pollfds[1].fd = ctx->tcp_fd;
	ctx->pollfds[1].events = POLLIN;

	// Make connection before setting the socket file descriptor to be non-blocking
	if(connect(ctx->tcp_fd, ctx->tcp_info->ai_addr, ctx->tcp_info->ai_addrlen) == -1) {
		print_log("Connection to server has failed: %d\n", errno);
		return -1;
	}

	opts = fcntl(ctx->pollfds[1].fd, F_GETFL, 0); // Get current fd options
	fcntl(ctx->tcp_fd, F_SETFL, opts | O_NONBLOCK); // Set non-blocking option

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
	ctx->pollfds[2].fd = ctx->udp_fd;
	ctx->pollfds[2].events = POLLIN;
	opts = fcntl(ctx->pollfds[2].fd, F_GETFL, 0);
	fcntl(ctx->udp_fd, F_SETFL, opts | O_NONBLOCK);

	return 0;
}

void close_client_network_context(client_network_context* ctx) {
	close(ctx->pollfds[0].fd); // IDK if we actually have to close stdin
	if(ctx->tcp_fd != -1) close(ctx->tcp_fd);
	if(ctx->udp_fd != -1) close(ctx->udp_fd);

	freeaddrinfo(ctx->tcp_info);
	freeaddrinfo(ctx->udp_info);
}
