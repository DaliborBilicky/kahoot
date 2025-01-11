#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stddef.h>
#include <sys/types.h>

#include "../sync_buffer/sync_buffer.h"

#define MAX_REQUEST_LEN 1024
#define MAX_RESPONSE_LEN 256
#define NUM_WORKERS 4
#define REQUEST_BUFFER_CAPACITY 100
#define RESPONSE_BUFFER_CAPACITY 100
#define MAX_PLAYERS 10

typedef struct Lobby {
    int id;
    int port;
    int current_players;
    int max_players;
    int clients[MAX_PLAYERS];
    struct Lobby *next;
} Lobby;

typedef struct ServerContext {
    SynchronizedBuffer request_buffer;
    SynchronizedBuffer response_buffer;
    char password[MAX_REQUEST_LEN];
    int passive_socket;
    pthread_t worker_threads[NUM_WORKERS];
    pthread_t response_thread;
    Lobby *lobbies;  
    pthread_t shutdown_thread;
    int port;
    atomic_bool running;
} ServerContext;

int server_init(ServerContext *context);
void server_run(ServerContext *context);
void server_shutdown(ServerContext *context);

int create_lobby(ServerContext *context, int port);
Lobby *get_lobby_by_id(ServerContext *context, int lobby_id);
void send_question_to_lobby(ServerContext *context, int lobby_id, const char *question);
#endif  // SERVER_H
