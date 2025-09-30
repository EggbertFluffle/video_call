#pragma once

#include "network.h"
#include "client.h"

/*
*	Print's the available commands to the console for reference
*/
int help_command();

/*
* 	Allocate and intialize the cleint_network_context for the ip address and port 
* 	spesified in argv
*/
int connect_command(client_application_context* ctx, int argc, char** argv);

/*
*  Send a disconnect packet to the server and free client_network_context
*/
int disconnect_command(client_application_context* ctx);

/*
*	Quit from the program, cleaning up the client_application_context beforehand
*/
int quit_command(client_application_context* ctx);

/*
* 	Set a variable for configuration in the application context using the following form
*	/set <variable> <value>
*/
int set_variable_command(client_application_context* ctx, int argc, char** argv);

/*
* 	Get a variable from the configuration in the application context using the following form
*	/get <variable>
*/
int get_variable_command(client_application_context* ctx, int argc, char** argv);
