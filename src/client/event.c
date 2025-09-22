#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "network.h"
#include "../common/log.h"
#include "../common/packets.h"

void handle_welcome_receive(client_network_context* ctx, client_packet* packet) {
	ctx->id = packet->payload.welcome.client_id;
}

void handle_chat_receive(client_packet* packet) {
	printf("%d: %s\n", packet->sender_id, packet->payload.chat.message);
}

int handle_chat_send(client_network_context* ctx, char* input_buffer, ssize_t input_length) {
	server_packet packet;
	packet.type = SERVER_CHAT_PAYLOAD;
	strncpy(packet.payload.chat.message, input_buffer, input_length);
	packet.payload.chat.size = input_length;

	print_log("Sending out client chat");

	ssize_t res;
	if((res = send(ctx->tcp_fd, (void*) &packet, sizeof(server_packet), 0)) == -1) {
		print_log("Error occured when sending chat");
		if(res != sizeof(server_packet)) {
			indent_print_log(1, "Bytes were lost in the sending. Sent %d/%d", res, sizeof(server_packet));
		}
		return -1;
	}

	return 0;
}

int handle_packet_receive(client_network_context* ctx) {
	client_packet packet;
	memset((void*) &packet, 0, sizeof(client_packet));

	if(ctx->tcp_pollfd->revents & POLLIN) {
		recv(ctx->tcp_fd, (void*) &packet, sizeof(client_packet), 0);
	}

	switch(packet.type) {
		case CLIENT_CHAT_PAYLOAD:
			handle_chat_receive(&packet);
			break;
		case CLIENT_WELCOME_PAYLOAD:
			handle_welcome_receive(ctx, &packet);
			break;
		case CLIENT_DISCONNECT_PAYLOAD:
			printf("Client %d disconnected\n", packet.sender_id);
			break;
	}

	return 0;
}

int text_input_handler(client_network_context* ctx) {
	char input_buffer[MESSAGE_BUFFER_LENGTH];

	if(ctx->tcp_pollfd->revents & POLLIN) {
		print_log("TEXT INPUT RECEIVED");
		ssize_t input_length = read(ctx->stdin_pollfd->fd, (void*) input_buffer, MESSAGE_BUFFER_LENGTH);

		if(input_length == -1) {
			return 0;
		}

		print_log("TEXT INPUT LENGTH: %d", input_length);

		bool is_command = input_buffer[0] == '/';

		if(is_command) {
			// handle_command(ctx, input_buffer, input_length);
		} else {
			print_log("handling chat");
			handle_chat_send(ctx, input_buffer, input_length);
		}
	}

	return 0;
}
