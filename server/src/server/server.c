#include "server.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../sockets/sockets.h"
#include "server_communication.h"

int server_init(ServerContext *context, int port, const char *password) {
    context->passive_socket = passive_socket_init(port, 5);
    if (context->passive_socket < 0) {
        return -1;
    }

    strncpy(context->password, password, MAX_REQUEST_LEN - 1);
    context->password[MAX_REQUEST_LEN - 1] = '\0';

    sync_buff_init(&context->request_buffer, REQUEST_BUFFER_CAPACITY,
                   sizeof(ClientMessage));
    sync_buff_init(&context->response_buffer, RESPONSE_BUFFER_CAPACITY,
                   sizeof(ClientMessage));

    return 0;
}

void server_run(ServerContext *context) {
    pthread_create(&context->response_thread, NULL, send_responses, context);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&context->worker_threads[i], NULL, process_requests,
                       context);
    }

    while (1) {
        int *active_socket = malloc(sizeof(int));
        if (!active_socket) {
            perror("ERROR: Failed to allocate memory for client socket");
            continue;
        }

        *active_socket = wait_for_client_connection(context->passive_socket);
        if (*active_socket < 0) {
            free(active_socket);
            continue;
        }

        RequestThreadData *data = malloc(sizeof(RequestThreadData));
        if (!data) {
            perror("ERROR: Failed to allocate memory for RequestThreadData");
            close(*active_socket);
            free(active_socket);
            continue;
        }

        data->active_socket = active_socket;
        data->context = context;

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_request, data);
        pthread_detach(client_thread);
    }
}

void server_shutdown(ServerContext *context) {
    pthread_join(context->response_thread, NULL);
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(context->worker_threads[i], NULL);
    }

    sync_buff_destroy(&context->request_buffer);
    sync_buff_destroy(&context->response_buffer);
    close(context->passive_socket);
}
