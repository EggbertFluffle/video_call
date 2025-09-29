#define _CLIENT

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

#include <poll.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>

#include "client.h"
#include "network.h"
#include "event.h"
#include "../common/utils.h"

client_application_context* application_context = NULL;

int main(void) {
	application_context = create_client_application_context();
	initialize_client_application_context(application_context);

	while(!application_context->quit) {
		handle_text_input(application_context);
	}

	exit(EXIT_SUCCESS);
	return 0;
}

client_application_context* create_client_application_context() {
	return (client_application_context*) malloc(sizeof(client_application_context));
}

void initialize_client_application_context(client_application_context* ctx) {
	ctx->network_context = NULL;
	ctx->quit = false;

	// Add stdin to the pollfd list
	ctx->stdin_pollfd.fd = STDIN_FILENO; // Add stdin to the poll list
	ctx->stdin_pollfd.events = POLLIN;

	set_nonblocking(ctx->stdin_pollfd.fd);

	memset((void*)&ctx->action, 0, sizeof(struct sigaction));
	ctx->action.sa_flags = 0;
	sigemptyset(&ctx->action.sa_mask);
	ctx->action.sa_handler = client_application_signal_handler;

	sigaction(SIGINT, &ctx->action, NULL);
	sigaction(SIGTERM, &ctx->action, NULL);

	atexit(client_application_exit_handler);
}

void close_client_application_context(client_application_context* ctx) {
	print_log("Closing client application context");

	if(ctx->network_context != NULL) {
		close_client_network_context(ctx->network_context);
	}

	free(ctx);
}

void client_application_exit_handler() {
	print_log("Exit handler running");

	if(application_context != NULL) {
		close_client_application_context(application_context);
	}
}

void client_application_signal_handler(int signum) {
	print_log("Exiting on signal %d", signum);

	exit(EXIT_FAILURE);
}

// int main(int argc, char** argv) {
// 	char server_ipv4[64];
// 	char server_port[32];
//
// 	if(argc < 3) {
// 		printf("What ip would you like to connect to?: ");
// 		scanf(" %s", server_ipv4);
//
// 		printf("What port would you like to connect through?: ");
// 		scanf(" %s", server_port);
// 	} else {
// 		strncpy(server_ipv4, argv[1], sizeof(server_ipv4));
// 		strncpy(server_port, argv[2], sizeof(server_port));
// 	}
//
// 	printf("%s\n", server_ipv4);
// 	printf("%s\n", server_port);
//
// 	if(initialize_client_network_context(&ctx, server_ipv4, server_port) == FAIL) {
// 		exit(FAIL);
// 	}
// 	
// 	bool exit = false;
// 	while(!exit) {
// 		poll_client_events(&ctx);
//
// 		text_input_handler(&ctx);
//
// 		handle_packet_receive(&ctx);
// 	}
//
//     // execute();
//
// 	close_client_network_context(&ctx);
// 	return 0;
// }
