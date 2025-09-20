#ifndef _VIDEO_CALL_PACKETS
#define _VIDEO_CALL_PACKETS

#define MESSAGE_BUFFER_LENGTH 255

typedef enum {
	CLIENT_CHAT_PAYLOAD,
	CLIENT_COMMAND_PAYLOAD,
	CLIENT_DISCONNECT_PAYLOAD
} client_payload_type;

typedef struct {
	char message[MESSAGE_BUFFER_LENGTH];
} client_chat_payload;

typedef struct {
	char command[MESSAGE_BUFFER_LENGTH];
} client_command_payload;

typedef union {
	client_chat_payload chat;
	client_command_payload command;
} client_packet_payload;

// Packet being sent FROM the client
typedef struct {
	int sender_id;
	client_payload_type type;
	client_packet_payload payload;
} client_packet;

#endif
