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

    ServerContext context;
    strncpy(context.password, password, MAX_REQUEST_LEN - 1);
    context.port = server_port;
    context.running = true;
    if (server_init(&context) < 0) {
        fprintf(stderr, "ERROR: Failed to initialize server\n");
        return EXIT_FAILURE;
    }

    server_run(&context);

    return EXIT_SUCCESS;
}
