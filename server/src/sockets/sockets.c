#include "sockets.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int passive_socket_init(const int server_port, int backlog) {
    int passive_socket;
    struct sockaddr_in server_address;

    passive_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (passive_socket < 0) {
        perror("ERROR: Failed to create passive socket");
        return -1;
    }

    memset((char *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    if (bind(passive_socket, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0) {
        perror("ERROR: Failed to bind passive socket");
        return -1;
    }

    if (listen(passive_socket, 5) < 0) {
        perror("ERROR: Failed to listen on passive socket");
        return -1;
    }

    printf("SERVER running PORT:%d\n", server_port);
    return passive_socket;
}

int wait_for_client_connection(int passive_socket) {
    struct sockaddr_in client_address;
    socklen_t client_size = sizeof(client_address);
    int active_socket = accept(
        passive_socket, (struct sockaddr *)&client_address, &client_size);

    if (active_socket < 0) {
        perror("ERROR: Failed to accept connection");
        return -1;
    }

    return active_socket;
}
