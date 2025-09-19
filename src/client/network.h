#ifndef _VIDEO_CALL_NETWORK
#define _VIDEO_CALL_NETWORK

#include <netdb.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

#include <poll.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MESSAGE_BUF_LEN 1024

typedef struct {
	struct pollfd pollfds[3];

	int sock_stream;
	struct addrinfo* sock_stream_info;

	int sock_dgram;
	struct addrinfo* sock_dgram_info;
} network_context;

/*
*	Initializes a poll file descriptor list for non-blocking interfacing with
*	stdin, steam socket to the server and a datagram socket to the server
*
*	@param netowkr_context* nctx: Network context to put initilization data in
*	@param char* server_ipv4: ipv4 of the desired server connection
*	@param char* server_port: go figure
* 	@return -1 on fail with log information and errno output, 0 otherwise
*/
int initialize_network_context(network_context* nctx, char* server_ipv4, char* server_port);

/*
*	Close the sockets inside of the network context for proper cleanup
*
*	@param netowkr_context* nctx: Network context to close the sockets for
*/
void close_network_context(network_context* nctx);

#endif
