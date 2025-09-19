CC= gcc
CFLAGS= -g -Wall -Werror -Wextra -Wpedantic

CLIENT_BINARY= ./bin/client
SERVER_BINARY= ./bin/server

CLIENT_OBJECTS= ./src/client/*.c ./src/common/*.c
SERVER_OBJECTS= ./src/server/*.c ./src/common/*.c

main: $(SERVER_BINARY) $(CLIENT_BINARY)

$(SERVER_BINARY): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BINARY): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm -f bin/client bin/server 
