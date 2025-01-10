#include "server.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../sockets/sockets.h"
#include "server_communication.h"

int server_init(ServerContext *context) {
    context->passive_socket = passive_socket_init(context->port);
    if (context->passive_socket < 0) {
        return -1;
    }

    context->password[MAX_REQUEST_LEN - 1] = '\0';

    sync_buff_init(&context->request_buffer, REQUEST_BUFFER_CAPACITY,
                   sizeof(ClientMessage), &context->running);
    sync_buff_init(&context->response_buffer, RESPONSE_BUFFER_CAPACITY,
                   sizeof(ClientMessage), &context->running);

    return 0;
}

void *shutdown_listener(void *arg) {
    ServerContext *context = (ServerContext *)arg;
    int command = 0;

    printf("Press CTRL+D do shutdown the server.\n");

    while (atomic_load(&context->running)) {
        if (scanf("%d", &command) == EOF) {
            printf("Shutdown command received.\n");
            atomic_store(&context->running, 0);

            int dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (dummy_socket < 0) {
                perror("ERROR: Failed to create dummy socket");
                return NULL;
            }

            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(context->port);
            server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

            if (connect(dummy_socket, (struct sockaddr *)&server_addr,
                        sizeof(server_addr)) < 0) {
                perror("ERROR: Failed to connect dummy socket");
                close(dummy_socket);
            } else {
                close(dummy_socket);
            }
        }
    }
    return NULL;
}

void server_run(ServerContext *context) {
    pthread_create(&context->response_thread, NULL, send_responses, context);
    pthread_create(&context->shutdown_thread, NULL, shutdown_listener, context);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&context->worker_threads[i], NULL, process_requests,
                       context);
    }

    while (atomic_load(&context->running)) {
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
    server_shutdown(context);
}

void server_shutdown(ServerContext *context) {
    sync_buffer_stop(&context->request_buffer);
    sync_buffer_stop(&context->response_buffer);

    pthread_join(context->response_thread, NULL);
    pthread_join(context->shutdown_thread, NULL);
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(context->worker_threads[i], NULL);
    }

    sync_buff_destroy(&context->request_buffer);
    sync_buff_destroy(&context->response_buffer);
    close(context->passive_socket);
}
