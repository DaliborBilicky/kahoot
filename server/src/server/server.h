#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdatomic.h>

#include "../lobby/lobby.h"
#include "../sync_buffer/sync_buffer.h"
#include "../sync_list/sync_list.h"

#define MAX_REQUEST_LEN 1024
#define MAX_RESPONSE_LEN 256
#define POOL_VOLUME 4
#define REQUEST_BUFFER_CAPACITY 100
#define RESPONSE_BUFFER_CAPACITY 100

typedef struct ServerContext {
    LobbyManager *lobby_manager;
    SynchronizedBuffer request_buffer;
    SynchronizedBuffer response_buffer;
    char password[MAX_REQUEST_LEN];
    int passive_socket;
    ThreadNode *thread_list_head;
    pthread_t thread_pool[POOL_VOLUME];
    pthread_t response_thread;
    pthread_t shutdown_thread;
    int port;
    atomic_bool running;
} ServerContext;

int server_init(ServerContext *self, LobbyManager *lobby_manager);
void server_run(ServerContext *self);
void server_shutdown(ServerContext *self);

#endif  // SERVER_H
