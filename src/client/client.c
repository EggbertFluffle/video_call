#define _CLIENT

#include <stdio.h>
#include <stdint.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

#include <poll.h>
#include <memory.h>
#include <unistd.h>

#include "network.h"
#include "camera.h"

network_context nctx;

int main(void) {

    execute();

	return 0;
}

int connect_init() {
	char *server_ipv4 = (char *)calloc(64, sizeof(char));
	char *server_port = (char *)calloc(16, sizeof(char));

	printf("What ip would you like to connect to?: ");
	scanf(" %s", server_ipv4);

	printf("What port would you like to connect through?: ");
	scanf(" %s", server_port);

	if(initialize_network_context(&nctx, server_ipv4, server_port) == -1) {
		exit(-1);
	}

    return 0;
}

