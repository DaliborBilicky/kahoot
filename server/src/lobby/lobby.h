#ifndef LOBBY_H
#define LOBBY_H

#include <pthread.h>
#include <stdatomic.h>

#include "../sync_list/sync_list.h"
#include "question.h"

#define MAX_NICKNAME_LEN 50
#define MAX_REQUEST_LEN 1024
#define MAX_RESPONSE_LEN 256

typedef struct Player {
    char *nick;
    int score;
} Player;

typedef struct Lobby {
    int id;
    int port;
    int current_players;
    int passive_socket;
    pthread_t admin_thread;
    ThreadNode *thread_list_head;
    atomic_bool running;
    SyncLinkedList players;
    SyncQuestion question;
} Lobby;

typedef struct LobbyManager {
    SyncLinkedList lobby_list;
    atomic_int lobby_id_counter;
} LobbyManager;

void lobby_manager_init(LobbyManager *self);
void lobby_manager_destroy(LobbyManager *self);
int lobby_manager_create_lobby(LobbyManager *self, int base_port);

int lobby_init(Lobby *self, int base_port, int lobby_id_counter);
void lobby_run(Lobby *self);
void lobby_shutdown(Lobby *self);

#endif  // LOBBY_H
