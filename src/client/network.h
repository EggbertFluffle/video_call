#pragma once

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

#include "../common/packets.h"

typedef struct {
	int id;

	int tcp_fd;
	struct addrinfo* tcp_info;
	struct pollfd* tcp_pollfd;

	int udp_fd;
	struct addrinfo* udp_info;
	struct pollfd* udp_pollfd;

	struct pollfd pollfds[2];
} client_network_context;

/*
*	Initializes a poll file descriptor list for non-blocking interfacing with
*	stdin, steam socket to the server and a datagram socket to the server
*
*	@param network_context* nctx: Network context to put initilization data in
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
 * Uses fcntl to set the fd to be nonblocking. Returns -1 on failure
 *
 * @param fd: filedescriptor to make nonblocking
 */
int set_nonblocking(int fd);

/*
*	Poll each pollable file descriptor for events before checking for events
*	to handle
*/
int poll_client_events(client_network_context* ctx);

/*
 * 	Send a packet to the server. Returns and prints errors to client log
 * 	
 * 	@param server_packet* packet: packet that will be sent to the server
 */
int send_packet(client_network_context* ctx, server_packet* packet);

/*
*	Read from stdin to get commands and chat messages from the user
*/
int text_input_handler(client_network_context* ctx);

/*
* 	Send a chat message to the server
*/
int handle_chat_send(client_network_context* ctx, char* input_buffer, ssize_t input_length);
