#ifndef LOBBY_H
#define LOBBY_H

#include <pthread.h>

#include "../sync_buffer/sync_buffer.c"
#include "../sync_list/sync_list.h"

#define MAX_NICKNAME_LEN 50

typedef struct Lobby {
    int id;
    int port;
    int current_players;
    int max_players;
    atomic_bool running;
    int passive_socket;
    pthread_t super_user_thread;
} Lobby;

typedef struct LobbyManager {
    SyncLinkedList lobby_list;
    int lobby_id_counter;
} LobbyManager;

typedef struct PlayerThreadData {
    Lobby *lobby;
    int *active_socket;
    int score;
    char nickname[MAX_NICKNAME_LEN];
} PlayerThreadData;

void lobby_manager_init(LobbyManager *self);
void lobby_manager_destroy(LobbyManager *self);
int lobby_manager_create_lobby(LobbyManager *self, int base_port);

int lobby_init(Lobby *self, int base_port, int lobby_id_counter);
void lobby_run(Lobby *self);
void lobby_shutdown(Lobby *self);

#endif  // LOBBY_H
