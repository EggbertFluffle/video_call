#ifndef _VIDEO_CALL_SERVER_NETWORK
#define _VIDEO_CALL_SERVER_NETWORK

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
#define MAX_CLIENTS 32
#define MAX_POLLFDS (MAX_CLIENTS + 3) // stdin, listen_tcp, udp

typedef struct {
	struct pollfd* pollfd;

	int id;
} client_connection;

typedef struct {
	struct pollfd* stdin_pollfd;

	int tcp_fd;
	struct pollfd* tcp_pollfd;
	struct addrinfo* tcp_info;

	int udp_fd;
	struct pollfd* udp_pollfd;
	struct addrinfo* udp_info;

	client_connection clients[MAX_CLIENTS];
	int client_count;

	struct pollfd pollfds[MAX_POLLFDS];
	int pollfd_count;
} server_network_context;

/*
*	Initializes a poll file descriptor list for non-blocking interfacing with
*	stdin, tcp socket and a udp socket
*
*	@param netowkr_context* nctx: Network context to put initilization data in
*	@param char* server_ipv4: ipv4 of the desired server connection
*	@param char* server_port: go figure
* 	@return -1 on fail with log information and errno output, 0 otherwise
*/
int initialize_server_network_context(server_network_context* ctx, char* server_ipv4, char* server_port);

/*
*	Close the sockets inside of the network context for proper cleanup
*
*	@param netowkr_context* nctx: Network context to close the sockets for
*/
void close_server_network_context(server_network_context* ctx);

/*
 * 	Accept any connections that appear on the server's listening tcp socket.
 * 	These connections are then added to the context's list of clients and thier 
 * 	newly opened sockets to the context's pollfds list
 */
int accept_client(server_network_context* ctx);

/*
 *	Removes the client who's client_id matches the given. Additionally closes
 *	connection and cleans up associated pollfd
 */
int remove_client(server_network_context* ctx, int client_id);

/*
* Poll all pollfds in the context for events before processing
*/
int poll_server_events(server_network_context* ctx);

#endif
