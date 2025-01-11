#ifndef LOBBY_H
#define LOBBY_H

#include "../sync_list/sync_list.h"

typedef struct Lobby {
    int id;
    int port;
    int current_players;
    int max_players;
} Lobby;

typedef struct LobbyManager {
    SyncLinkedList lobby_list;
} LobbyManager;

void lobby_init(LobbyManager *self);
void lobby_destroy(LobbyManager *self);

int create_lobby(LobbyManager *self, int server_port);
LobbyManager *get_lobby_porc_by_id(LobbyManager *self, int lobby_id);
void handle_lobby(LobbyManager *self, int lobby_id);

#endif  // LOBBY_H
