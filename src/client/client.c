#define _CLIENT

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

#include <poll.h>
#include <memory.h>
#include <unistd.h>

#include "network.h"
#include "event.h"
#include "../common/utils.h"

client_network_context ctx;

int main(void) {
	char *server_ipv4 = (char *)calloc(64, sizeof(char));
	char *server_port = (char *)calloc(16, sizeof(char));

	printf("What ip would you like to connect to?: ");
	scanf(" %s", server_ipv4);

	printf("What port would you like to connect through?: ");
	scanf(" %s", server_port);

	if(initialize_client_network_context(&ctx, server_ipv4, server_port) == FAIL) {
		exit(FAIL);
	}
	free(server_ipv4);
	free(server_port);
	
	bool exit = false;
	while(!exit) {
		poll_client_events(&ctx);

		text_input_handler(&ctx);

		handle_packet_receive(&ctx);
	}

    // execute();

	close_client_network_context(&ctx);
	return 0;
}
