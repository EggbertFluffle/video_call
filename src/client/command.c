#define _CLIENT

#include "command.h"
#include "client.h"
#include "network.h"
#include "../common/packets.h"
#include "../common/log.h"

int client_connect_command(client_application_context* ctx, int argc, char** argv) {
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

int client_disconnect_command(client_application_context* ctx) {
	if(ctx->network_context == NULL) {
		print_log("Not currently connected to a server");
		return -1;
	}

	close_client_network_context(ctx->network_context);

	ctx->network_context = NULL;

	return 0;
}

int client_quit_command(client_application_context* ctx) {

	// If still connected to a server, disconnect properly
	if(ctx->network_context != NULL) {
		client_disconnect_command(ctx);
	}

	ctx->quit = true;

	return 0;
}
