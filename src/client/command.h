#pragma once

#include "network.h"
#include "client.h"

/*
* 	Allocate and intialize the cleint_network_context for the ip address and port 
* 	spesified in argv
*/
int client_connect_command(client_application_context* ctx, int argc, char** argv);

/*
*  Send a disconnect packet to the server and free client_network_context
*/
int client_disconnect_command(client_application_context* ctx);

/*
*	Quit from the program, cleaning up the client_application_context beforehand
*/
int client_quit_command(client_application_context* ctx);
