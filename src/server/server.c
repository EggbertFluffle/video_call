#include <string.h>
#include <sys/socket.h>
#define _SERVER

#include <stdio.h>
#include <stdbool.h>
#include <memory.h>

#include "network.h"
#include "../common/packets.h"
#include "../common/log.h"

#define MESSAGE_BUF_LEN 1024

server_network_context ctx;

int main(void) {
	print_log("Hello, world!");

	char *server_ipv4 = (char *)calloc(64, sizeof(char));
	char *server_port = (char *)calloc(16, sizeof(char));

	printf("What ip will you connect from?: ");
	scanf(" %s", server_ipv4);

	printf("What port would you like to connect through?: ");
	scanf(" %s", server_port);

	if(initialize_server_network_context(&ctx, server_ipv4, server_port) == -1) {
		exit(-1);
	}

	const char* face_message = "+-----+\n|     |\n| o o |\n|  ^  |\n+-----+";

	bool exit = false;
	while(!exit) {
		poll_server_events(&ctx);

		accept_client(&ctx);

		for(int c = 0; c < ctx.client_count; c++) {
			client_packet packet;
			packet.type = CLIENT_CHAT_PAYLOAD;
			strncpy(packet.payload.chat.message, face_message, strlen(face_message));

			if(send(ctx.clients[c].pollfd->fd, (void*) &packet, sizeof(client_packet), 0) == -1) {
				print_log("Couldn't send packet to %d", ctx.clients[c].id);
			}
		}
	}
}
