#include "event.h"
#include <ctype.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>

#include "client.h"
#include "network.h"
#include "command.h"
#include "../common/log.h"
#include "../common/packets.h"

void handle_welcome_receive(client_network_context* ctx, client_packet* packet) {
	ctx->id = packet->payload.welcome.client_id;
	print_log("Received welcome packet, new id is %d", ctx->id);
}

void handle_chat_receive(client_packet* packet) {
	if(packet->sender_id == 0) {
		print_log("Receiving chat packet form id 0");
		return;
	}
	printf("%d: %s", packet->sender_id, packet->payload.chat.message);
}

int handle_chat_send(client_network_context* ctx, char* input_buffer, ssize_t input_length) {
	server_packet packet;
	packet.sender_id = ctx->id;
	packet.type = SERVER_CHAT_PAYLOAD;
	strncpy(packet.payload.chat.message, input_buffer, input_length);
	packet.payload.chat.size = input_length;

	print_log("Sending out client chat");

	ssize_t res = send(ctx->tcp_fd, (void*) &packet, sizeof(server_packet), 0);
	if(res < 0) {
		print_log("Error occured when sending chat");
		if(res != sizeof(server_packet)) {
			indent_print_log(1, "Bytes were lost in the sending. Sent %d/%d", res, sizeof(server_packet));
		}
		return -1;
	}

	return 0;
}

int handle_command(client_application_context *ctx, char *input_buffer, ssize_t input_length) {
	int argc = 0;
	char* argv[COMMAND_ARGUMENT_MAX];
	bool inside_argument = false;

	input_buffer[input_length - 1] = '\0';

	// populate argc and argv by replacing spaces with null terminators
	for(int i = 1; i < input_length && argc < COMMAND_ARGUMENT_MAX; i++) {
		if(inside_argument) {
			// If this is the first space, make null terminator and prepare for next argument
			if(isblank(input_buffer[i])) {
				input_buffer[i] = '\0';
				inside_argument = false;
			}
		} else {
			if(isalnum(input_buffer[i])) {
				argv[argc] = &input_buffer[i];
				argc++;
				inside_argument = true;
			}
		}
	}

	if(argc == 0) {
		print_log("No command received");
		return -1;
	}

	if(strncmp(argv[0], "quit", strnlen(argv[0], 4)) == 0 || 
		strncmp(argv[0], "exit", strnlen(argv[0], 4)) == 0) {
		quit_command(ctx);
	} else if(strncmp(argv[0], "connect", strnlen(argv[0], 7)) == 0) {
		connect_command(ctx, argc, argv);
	} else if(strncmp(argv[0], "disconnect", strnlen(argv[0], 10)) == 0) {
		disconnect_command(ctx);
	} else if(strncmp(argv[0], "help", strnlen(argv[0], 4)) == 0) {
		help_command();
	} else if(strncmp(argv[0], "set", strnlen(argv[0], 3)) == 0) {
		set_variable_command(ctx, argc, argv);
	} else if(strncmp(argv[0], "get", strnlen(argv[0], 3)) == 0) {
		get_variable_command(ctx, argc, argv);
	} else {
		print_log("Command \"%s\" does not exist", argv[0]);
		return -1;
	}
	
	return 0;
}

int handle_packet_receive(client_network_context* ctx) {
	if (ctx->tcp_pollfd->revents & (POLLHUP | POLLNVAL | POLLERR)) {
		print_log("Server has hungup, became invalid, or errored, removing connection");
		indent_print_log(1, "Events result:");
		indent_print_log(2, "POLLHUP: %s", ctx->tcp_pollfd->revents & POLLHUP ? "true" : "false");
		indent_print_log(2, "POLLNVAL: %s", ctx->tcp_pollfd->revents & POLLNVAL ? "true" : "false");
		indent_print_log(2, "POLLERR: %s", ctx->tcp_pollfd->revents & POLLERR ? "true" : "false");

		return -1;
	}

	if(!(ctx->tcp_pollfd->revents & POLLIN)) {
		return 0;
	}

	client_packet packet;
	ssize_t bytes = recv(ctx->tcp_fd, (void*) &packet, sizeof(client_packet), 0);

	if(bytes == 0) {
		print_log("Server closed connection gracefully");
		return -1;
	} else if(bytes < 0){
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			print_log("No packet available to read");
			return 0;
		} else if(errno == EINTR) {
			print_log("Packet read was interrupted, retrying");
			return handle_packet_receive(ctx);
		}
		
		print_log("Unable to read packet from server");
		return 0;
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
			//TODO: SHOULD USE A HANDLER
			// handle_disconnect_receive(ctx, &packet);
			break;
	}

	return 0;
}

int handle_text_input(client_application_context* ctx) {
	char input_buffer[MESSAGE_BUFFER_LENGTH];
	memset((void*)input_buffer, 0, MESSAGE_BUFFER_LENGTH);

	int fd_count = poll(&ctx->stdin_pollfd, 1, 0);

	if(fd_count < 0) {
		print_log("Unable to poll stdin for text input");
		return -1;
	}

	if(ctx->stdin_pollfd.revents & POLLIN) {
		ssize_t input_length = read(ctx->stdin_pollfd.fd, (void*) input_buffer, MESSAGE_BUFFER_LENGTH);

		if(input_length == -1) {
			return 0;
		}

		bool is_command = input_buffer[0] == '/';

		if(is_command) {
			handle_command(ctx, input_buffer, input_length);
		} else {
			if(ctx->network_context == NULL) {
				print_log("Connect to a server using \"/connect <ip addr> <port> to send chat messages");
				return -1;
			}
			handle_chat_send(ctx->network_context, input_buffer, input_length);
		}
	}

	return 0;
}

