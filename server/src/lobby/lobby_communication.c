#include "lobby_communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void* handle_admin(void* arg) {
    LobbyThreadData* data = (LobbyThreadData*)arg;
    Lobby* lobby = data->lobby;
    int active_socket = *(data->active_socket);

    free(data->active_socket);
    free(data);

    printf("IN admin thread\n");
    send(active_socket, "LOBBY_JOINED", 12, 0);

    close(active_socket);
    return NULL;
}

void* handle_player(void* arg) {
    LobbyThreadData* data = (LobbyThreadData*)arg;
    Lobby* lobby = data->lobby;
    int active_socket = *(data->active_socket);
    char* nick = strdup(data->nick);

    free(data->active_socket);
    free(data);

    printf("IN player thread\n");

    close(active_socket);
    return NULL;
}