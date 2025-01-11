#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#include "server/server.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <port> <password>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);
    const char *password = argv[2];

    LobbyManager lobby_manager;
    ServerContext server;
    strncpy(server.password, password, MAX_REQUEST_LEN - 1);
    server.port = server_port;
    if (server_init(&server, &lobby_manager) < 0) {
        fprintf(stderr, "ERROR: Failed to initialize server\n");
        return EXIT_FAILURE;
    }

    server_run(&server);

    return EXIT_SUCCESS;
}
