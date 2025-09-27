#include <string.h>
#define _SERVER

#include <sys/socket.h>
#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

#include "event.h"
#include "network.h"

#include "../common/log.h"

#define MESSAGE_BUF_LEN 1024

server_network_context ctx;

int main(int argc, char** argv) {
	char server_ipv4[64];
	char server_port[32];

	if(argc < 3) {
		printf("What ip will you connect from?: ");
		scanf(" %s", server_ipv4);

		printf("What port would you like to connect through?: ");
		scanf(" %s", server_port);
	} else {
		strncpy(server_ipv4, argv[1], sizeof(server_ipv4));
		strncpy(server_port, argv[2], sizeof(server_port));
	}

	printf("%s\n", server_ipv4);
	printf("%s\n", server_port);

	if(initialize_server_network_context(&ctx, server_ipv4, server_port) == -1) {
		exit(-1);
	}

	// const char* face_message = "+-----+\n|     |\n| o o |\n|  ^  |\n+-----+";

	bool exit = false;
	while(!exit) {
		poll_server_events(&ctx);
		accept_client(&ctx);

		handle_packet_receive(&ctx);
	}
}
