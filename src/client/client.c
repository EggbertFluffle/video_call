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

int main(int argc, char** argv) {
	char server_ipv4[64];
	char server_port[32];

	if(argc < 3) {
		printf("What ip would you like to connect to?: ");
		scanf(" %s", server_ipv4);

		printf("What port would you like to connect through?: ");
		scanf(" %s", server_port);
	} else {
		strncpy(server_ipv4, argv[1], sizeof(server_ipv4));
		strncpy(server_port, argv[2], sizeof(server_port));
	}

	printf("%s\n", server_ipv4);
	printf("%s\n", server_port);

	if(initialize_client_network_context(&ctx, server_ipv4, server_port) == FAIL) {
		exit(FAIL);
	}
	
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
