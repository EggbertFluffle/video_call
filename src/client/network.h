#ifndef _VIDEO_CALL_CLIENT_NETWORK
#define _VIDEO_CALL_CLIENT_NETWORK

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

typedef struct {
	struct pollfd* stdin_pollfd;

	int tcp_fd;
	struct addrinfo* tcp_info;
	struct pollfd* tcp_pollfd;

	int udp_fd;
	struct addrinfo* udp_info;
	struct pollfd* udp_pollfd;

	struct pollfd pollfds[3];
} client_network_context;

/*
*	Initializes a poll file descriptor list for non-blocking interfacing with
*	stdin, steam socket to the server and a datagram socket to the server
*
*	@param netowkr_context* nctx: Network context to put initilization data in
*	@param char* server_ipv4: ipv4 of the desired server connection
*	@param char* server_port: go figure
* 	@return -1 on fail with log information and errno output, 0 otherwise
*/
int initialize_client_network_context(client_network_context* ctx, char* server_ipv4, char* server_port);

/*
*	Close the sockets inside of the network context for proper cleanup
*
*	@param netowkr_context* nctx: Network context to close the sockets for
*/
void close_client_network_context(client_network_context* ctx);

/*
 *	Poll each pollable file descriptor for events before checking for events
 *	to handle
 */
int poll_client_events(client_network_context* ctx);

/*
*	Receive tcp packets from server if they are present. Handle each packet
*	type accordingly.
*/
int event_packet_handler(client_network_context* ctx);

#endif
