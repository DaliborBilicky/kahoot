#include <stdio.h>

#include "server/server.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <port> <password>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_port = atoi(argv[1]);
    const char *password = argv[2];

    ServerContext context;
    if (server_init(&context, server_port, password) < 0) {
        fprintf(stderr, "ERROR: Failed to initialize server\n");
        return EXIT_FAILURE;
    }

    server_run(&context);
    server_shutdown(&context);

    return EXIT_SUCCESS;
}
