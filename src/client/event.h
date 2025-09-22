#pragma once

#include <string.h>
#include <sys/socket.h>

#include "network.h"
#include "../common/log.h"
#include "../common/packets.h"

void handle_welcome_receive(client_network_context* ctx, client_packet* packet);

void handle_chat_receive(client_packet* packet);

int handle_chat_send(client_network_context* ctx, char* input_buffer, ssize_t input_length);

/*
*	Receive tcp packets from server if they are present. Handle each packet
*	type accordingly.
*/
int handle_packet_receive(client_network_context* ctx);

int text_input_handler(client_network_context* ctx);
