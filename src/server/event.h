#pragma once

#include "network.h"

/*
*	Receive a chat packet, constuct a client chat packet, and send to all clients
*	sans the sender
*/
int handle_chat_receive(server_network_context* ctx, server_packet* packet);

/*
* 	Receive any packet and call the corresponding event based on the packet's
* 	payload type
*/
int handle_packet_receive(server_network_context *ctx);
