#ifndef LOBBY_COMMUNICATION_H
#define LOBBY_COMMUNICATION_H

#include "lobby.h"

typedef struct LobbyThreadData {
    Lobby* lobby;
    int* active_socket;
} LobbyThreadData;

void* handle_admin(void* arg);

void* handle_player(void* arg);

#endif  // LOBBY_COMMUNICATION_H
