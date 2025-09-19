CC= gcc
CFLAGS= -g -Wall -Werror -Wextra -Wpedantic

CLIENT_BINARY= ./bin/client
SERVER_BINARY= ./bin/server

main: $(SERVER_BINARY) $(CLIENT_BINARY)

$(SERVER_BINARY): ./src/server/server.c
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BINARY): ./src/client/client.c ./src/client/network.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm -f bin/client bin/server 
