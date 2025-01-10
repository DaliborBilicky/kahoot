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

typedef struct ServerContext {
    SynchronizedBuffer request_buffer;
    SynchronizedBuffer response_buffer;
    char password[MAX_REQUEST_LEN];
    int passive_socket;
    pthread_t worker_threads[NUM_WORKERS];
    pthread_t response_thread;
    pthread_t shutdown_thread;
    int port;
    atomic_bool running;
} ServerContext;

int server_init(ServerContext *self);
void server_run(ServerContext *self);
void server_shutdown(ServerContext *self);

#endif  // SERVER_H
