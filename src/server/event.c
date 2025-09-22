#include "network.h"
#include "event.h"
#include "../common/packets.h"
#include "../common/log.h"

int handle_chat_receive(server_network_context* ctx, server_packet* packet) {
	client_packet cp;
	cp.type = CLIENT_CHAT_PAYLOAD;
	cp.payload.chat.size = packet->payload.chat.size;
	memcpy(cp.payload.chat.message, packet->payload.chat.message, cp.payload.chat.size);

	print_log("Handling server chat packet and sending out");

	if(send_packet(ctx, -packet->sender_id, &cp) == -1) {
		print_log("Broadcasting chat packet error");
	}

	return -1;
}

int handle_packet_receive(server_network_context *ctx) {
	for(int c = 0; c < ctx->client_count; c++) {
		if(ctx->clients[c].pollfd->events & POLLIN) {
			server_packet packet;

			if(read(ctx->clients[c].pollfd->fd, (void*) &packet, sizeof(server_packet)) == -1) {
				print_log("Unable to read packet from client %d", ctx->clients[c].id);
			}

			print_log("Received packet from client %d", packet.sender_id);

			switch(packet.type) {
				case SERVER_CHAT_PAYLOAD:
					handle_chat_receive(ctx, &packet);
					break;
				case SERVER_COMMAND_PAYLOAD:
					// handle_command();
					break;
			}
		}
	}

	return 0;
}
