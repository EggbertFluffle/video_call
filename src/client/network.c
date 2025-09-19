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

int initialize_network_context(network_context* nctx, char* server_ipv4, char* server_port) {
	nctx->pollfds[0].fd = 0; // Add stdin to the poll list
	nctx->pollfds[0].events = POLLIN;
	int opts = fcntl(nctx->pollfds[0].fd, F_GETFL, 0); // Get the current socket options
	fcntl(nctx->pollfds[0].fd, F_SETFL, opts | O_NONBLOCK); // Stdin should be non-blocking


	// Get address info for server domain/ip and port
	int status = 0;
	struct addrinfo sock_stream_hints;

	// Get addres info of the server
	memset(&sock_stream_hints, 0, sizeof(sock_stream_hints));
	sock_stream_hints.ai_family = AF_INET;
	sock_stream_hints.ai_socktype = SOCK_STREAM;
	if((status = getaddrinfo(server_ipv4, server_port, &sock_stream_hints, &nctx->sock_stream_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		return -1;
	}

	// Create a TCP socket to connect to the server with
	nctx->sock_stream = socket(nctx->sock_stream_info->ai_family, nctx->sock_stream_info->ai_socktype, nctx->sock_stream_info->ai_protocol); // Create server socket stream
	if(nctx->sock_stream == -1) {
		print_log("Unable to create socket stream for server! Errno: %d", errno);
		return -1;
	}
	nctx->pollfds[1].fd = nctx->sock_stream;
	nctx->pollfds[1].events = POLLIN;
	opts = fcntl(nctx->pollfds[1].fd, F_GETFL, 0);
	fcntl(nctx->sock_stream, F_SETFL, opts | O_NONBLOCK);

	if(connect(nctx->sock_stream, nctx->sock_stream_info->ai_addr, nctx->sock_stream_info->ai_addrlen) == -1) {
		print_log("Connection to server has failed: %d\n", errno);
		return -1;
	}

	// Repeat the process for the UDP DATAGRAM_SOCKET
	struct addrinfo sock_dgram_hints;
	memset(&sock_dgram_hints, 0, sizeof(sock_dgram_hints));
	sock_dgram_hints.ai_family = AF_INET;
	sock_dgram_hints.ai_socktype = SOCK_DGRAM;
	if((status = getaddrinfo(server_ipv4, server_port, &sock_dgram_hints, &nctx->sock_dgram_info)) != 0) {
		print_log("Get address info error: %s\n", gai_strerror(status));
		return -1;
	}

	nctx->sock_dgram = socket(nctx->sock_dgram_info->ai_family, nctx->sock_dgram_info->ai_socktype, nctx->sock_dgram_info->ai_protocol); // Create server socket datagram
	if(nctx->sock_dgram == -1) {
		print_log("Unable to create socket datagram for server! Errno: %d", errno);
		return -1;
	}
	nctx->pollfds[2].fd = nctx->sock_dgram;
	nctx->pollfds[2].events = POLLIN;
	opts = fcntl(nctx->pollfds[2].fd, F_GETFL, 0);
	fcntl(nctx->sock_dgram, F_SETFL, opts | O_NONBLOCK);

	return 0;
}

void close_network_context(network_context* nctx) {
	close(nctx->pollfds[0].fd); // IDK if we actually have to close stdin
	if(nctx->sock_stream != -1) close(nctx->sock_stream);
	if(nctx->sock_dgram != -1) close(nctx->sock_dgram);

	freeaddrinfo(nctx->sock_stream_info);
	freeaddrinfo(nctx->sock_dgram_info);
}
