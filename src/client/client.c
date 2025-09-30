#define _CLIENT

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

#include <poll.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>

#include "command.h"
#include "client.h"
#include "network.h"
#include "event.h"

client_application_context* application_context = NULL;

int main(void) {
	application_context = create_application_context();
	initialize_application_context(application_context);

	while(!application_context->quit) {
		handle_text_input(application_context);

		if(application_context->network_context != NULL && 
			client_network_update(application_context->network_context) == -1) {
			close_client_network_context(application_context->network_context);
			application_context->network_context = NULL;
		}
	}

	exit(EXIT_SUCCESS);
	return 0;
}

client_application_context* create_application_context() {
	return (client_application_context*) malloc(sizeof(client_application_context));
}

void initialize_application_context(client_application_context* ctx) {
	ctx->network_context = NULL;
	ctx->quit = false;

	memset((void*)ctx->username, 0, USERNAME_BUFFER_LENGTH);
	strncpy(ctx->username, "anonymous", USERNAME_BUFFER_LENGTH);

	// Add stdin to the pollfd list
	ctx->stdin_pollfd.fd = STDIN_FILENO; // Add stdin to the poll list
	ctx->stdin_pollfd.events = POLLIN;

	set_nonblocking(ctx->stdin_pollfd.fd);

	memset((void*)&ctx->action, 0, sizeof(struct sigaction));
	ctx->action.sa_flags = 0;
	sigemptyset(&ctx->action.sa_mask);
	ctx->action.sa_handler = signal_handler;

	sigaction(SIGINT, &ctx->action, NULL);
	sigaction(SIGTERM, &ctx->action, NULL);

	atexit(exit_handler);

	help_command();
}

void close_application_context(client_application_context* ctx) {
	if(ctx->network_context != NULL) {
		close_client_network_context(ctx->network_context);
	}

	free(ctx);
}

void exit_handler() {
	if(application_context != NULL) {
		close_application_context(application_context);
	}
}

void signal_handler(int signum) {
	print_log("Received signal %d", signum);

	exit(EXIT_FAILURE);
}
