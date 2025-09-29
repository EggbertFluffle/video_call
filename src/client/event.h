#pragma once

#include <string.h>
#include <sys/socket.h>

#include "network.h"
#include "client.h"
#include "../common/log.h"
#include "../common/packets.h"

#define COMMAND_ARGUMENT_MAX 32

void handle_welcome_receive(client_network_context* ctx, client_packet* packet);

void handle_chat_receive(client_packet* packet);

int handle_chat_send(client_network_context* ctx, char* input_buffer, ssize_t input_length);

int handle_command(client_application_context* ctx, char* input_buffer, ssize_t input_length);

/*
*	Receive tcp packets from server if they are present. Handle each packet
*	type accordingly.
*/
int handle_packet_receive(client_network_context* ctx);

/*
* Received text input from non-blocking stdin and classifies as chat message or command.
* Calls the corresponding handler for each.
*/
int handle_text_input(client_application_context* ctx);
