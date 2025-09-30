#pragma once

#include <signal.h>
#include <stdbool.h> // We arguably don't need this

#include "../common/packets.h"
#include "network.h"

typedef struct {
	client_network_context* network_context;
	bool quit;
	struct sigaction action;

	struct pollfd stdin_pollfd;
	char username[USERNAME_BUFFER_LENGTH];
} client_application_context;

/*
* 	Allocate memory for the client_application_context
*/
client_application_context* create_application_context();

/*
*	Initialize the client_application_context and as well as the program's
*	atexit functions and signal handlers to call close_client_application_context
*/
void initialize_application_context(client_application_context* ctx);

/*
*	Clean up memory from client_application_context
*/
void close_application_context(client_application_context* ctx);

/*
*	Exit handler for the client application context given to atexit()
*/
void exit_handler();

/*
* 	Signal handler for the client application context using sigaction()
*/
void signal_handler(int signum);
