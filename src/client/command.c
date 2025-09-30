#include <string.h>
#define _CLIENT

#include "command.h"
#include "client.h"
#include "network.h"
#include "../common/log.h"
#include "../common/packets.h"
#include "../common/log.h"

int help_command() {
	print_log("Welcome! Below are some of the commands you can run to get started.");
	indent_print_log(1, "/help - Prints this help menu for command reference");
	indent_print_log(1, "/exit - Closes the application, first disconnect if a connection to a connection is active");
	indent_print_log(1, "/quit- ^ ^ ^");
	indent_print_log(1, "/connect <ip> <port> - Start a connection to a server, with the given ip and port");
	indent_print_log(1, "/disconnect - Disconnect from a server if a connection exists");
	indent_print_log(1, "/set <variable> <value> - Sets a variable to the specified value");
	indent_print_log(1, "/get <variable> - Get the value of a variable, or all variables if given no args");

	return 0;
}

int connect_command(client_application_context* ctx, int argc, char** argv) {
	if(argc < 3) {
		print_log("Not enough arguments provided to command \"/connect <ip addr> <port>\"");
		return -1;
	}

	if(ctx->network_context != NULL) {
		print_log("Connection to a server is still made, run \"/disconnect\" first");
		return -1;
	}

	print_log("ip addr: %s", argv[1]);
	print_log("port: %s", argv[2]);

	ctx->network_context = (client_network_context*) malloc(sizeof(client_network_context));
	initialize_client_network_context(ctx->network_context, argv[1], argv[2]);

	return 0;
}

int disconnect_command(client_application_context* ctx) {
	if(ctx->network_context == NULL) {
		print_log("Not currently connected to a server");
		return -1;
	}

	close_client_network_context(ctx->network_context);

	ctx->network_context = NULL;

	return 0;
}

int quit_command(client_application_context* ctx) {

	// If still connected to a server, disconnect properly
	if(ctx->network_context != NULL) {
		disconnect_command(ctx);
	}

	ctx->quit = true;

	return 0;
}

int set_variable_command(client_application_context* ctx, int argc, char** argv) {
	if(argc < 3) {
		print_log("Not enough arguments provided to command \"/set <variable> <value>\"");
		return -1;
	}

	if(strncmp(argv[1], "username", strnlen(argv[1], 8)) == 0) {
		memset((void*)ctx->username, 0, USERNAME_BUFFER_LENGTH);
		strncpy(ctx->username, argv[2], strlen(argv[2]));
	} else {
		print_log("Variable name \"%s\" does not exist", argv[1]);
		return -1;
	}

	return 0;
}

int get_variable_command(client_application_context* ctx, int argc, char** argv) {
	if(argc == 2) {
		// Spesific variable return
		if(strncmp(argv[1], "username", strnlen(argv[1], 8)) == 0) {
			print_log("username: %s", ctx->username);
		} else {
			print_log("Variable name \"%s\" does not exist", argv[1]);
			return -1;
		}
	} else if(argc == 1) {
		// Print all variables
		print_log("username: %s", ctx->username);
	}
	

	return 0;
}
