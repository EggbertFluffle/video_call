#include "network.h"
#include "event.h"
#include "../common/packets.h"
#include "../common/log.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <sys/poll.h>

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
	for(int c = 0; c < MAX_CLIENTS; c++) {
		client_connection* client = &ctx->clients[c];

		if(client->id == -1) {
			continue;
		}

		if (client->pollfd->revents & (POLLHUP | POLLNVAL | POLLERR)) {
			print_log("Client %d:%d has hungup, became invalid, or errored, removing connection", client->id, c);
			indent_print_log(1, "Events result:");
			indent_print_log(2, "POLLHUP: %s", client->pollfd->revents & POLLHUP ? "true" : "false");
			indent_print_log(2, "POLLNVAL: %s", client->pollfd->revents & POLLNVAL ? "true" : "false");
			indent_print_log(2, "POLLERR: %s", client->pollfd->revents & POLLERR ? "true" : "false");

			remove_client(ctx, client->id);
			continue;
		}

		if(!(client->pollfd->revents & POLLIN)) {
			continue;
		}

		server_packet packet;
		ssize_t bytes = read(client->pollfd->fd, (void*) &packet, sizeof(server_packet));

		if(bytes == 0) {
			print_log("Client %d closed connection gracefully");
			remove_client(ctx, client->id);
			continue;
		} else if (bytes < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
				print_log("No packet available to read");
				return 0;
			} else if(errno == EINTR) {
				print_log("Packet read was interrupted, retrying");
				c--;
				continue;
			}

			print_log("Unable to read packet from client %d in pos %d", client->id, c);
			continue;
		}
		print_log("Received packet from client %d(%s)", packet.sender_id);

		switch(packet.type) {
			case SERVER_CHAT_PAYLOAD:
				handle_chat_receive(ctx, &packet);
				break;
			case SERVER_DISCONNECT_PAYLOAD:
				remove_client(ctx, packet.sender_id);
				break;
		}
	}

	return 0;
}
