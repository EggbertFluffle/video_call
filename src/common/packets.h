#pragma once

#define MESSAGE_BUFFER_LENGTH 255

#include <string.h>

//--------------------------CLIENT PACKETS--------------------------//

typedef enum {
	CLIENT_CHAT_PAYLOAD,
	CLIENT_WELCOME_PAYLOAD,	
	CLIENT_DISCONNECT_PAYLOAD
} client_payload_type;

typedef struct {
	size_t size;
	char message[MESSAGE_BUFFER_LENGTH];
} client_chat_payload;

typedef struct {
	int client_id;
} client_welcome_payload;

typedef union {
	client_chat_payload chat;
	client_welcome_payload welcome;
} client_payload;

// Packet being sent FROM the client
typedef struct {
	int sender_id;
	client_payload_type type;
	client_payload payload;
} client_packet;

//--------------------------SERVER PACKETS--------------------------//

typedef enum {
	SERVER_CHAT_PAYLOAD,
	SERVER_COMMAND_PAYLOAD
} server_payload_type;

typedef struct {
	size_t size;
	char message[MESSAGE_BUFFER_LENGTH];
} server_chat_payload;

typedef struct {
	char message[MESSAGE_BUFFER_LENGTH];
} server_command_payload;

typedef union {
	server_chat_payload chat;
	server_command_payload command;
} server_payload;

typedef struct {
	int sender_id;
	server_payload_type type;
	server_payload payload;
} server_packet;
