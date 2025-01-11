#include "server.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../lobby/lobby.h"
#include "../sockets/sockets.h"
#include "server_communication.h"

void append_thread_to_list(ServerContext *self, pthread_t thread) {
    ThreadNode *new_node = malloc(sizeof(ThreadNode));
    if (!new_node) {
        perror("ERROR: Failed to allocate memory for thread node");
        return;
    }
    new_node->thread_id = thread;
    new_node->next = NULL;

    if (self->thread_list_head == NULL) {
        self->thread_list_head = new_node;
    } else {
        ThreadNode *current = self->thread_list_head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void join_all_threads(ServerContext *self) {
    ThreadNode *current = self->thread_list_head;
    while (current) {
        pthread_join(current->thread_id, NULL);
        ThreadNode *to_free = current;
        current = current->next;
        free(to_free);
    }
    self->thread_list_head = NULL;
}

int server_init(ServerContext *self) {
    self->passive_socket = passive_socket_init(self->port);
    if (self->passive_socket < 0) {
        return -1;
    }

    self->password[MAX_REQUEST_LEN - 1] = '\0';

    sync_buff_init(&self->request_buffer, REQUEST_BUFFER_CAPACITY,
                   sizeof(ClientMessage), &self->running);
    sync_buff_init(&self->response_buffer, RESPONSE_BUFFER_CAPACITY,
                   sizeof(ClientMessage), &self->running);

    self->thread_list_head = NULL;
    print_hi();

    return 0;
}

void *shutdown_listener(void *arg) {
    ServerContext *self = (ServerContext *)arg;
    int command = 0;

    printf("Press CTRL+D do shutdown the server.\n");

    while (atomic_load(&self->running)) {
        if (scanf("%d", &command) == EOF) {
            printf("Shutdown command received.\n");
            atomic_store(&self->running, 0);

            int dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (dummy_socket < 0) {
                perror("ERROR: Failed to create dummy socket");
                return NULL;
            }

            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(self->port);
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

void server_run(ServerContext *self) {
    pthread_create(&self->response_thread, NULL, send_responses, self);
    pthread_create(&self->shutdown_thread, NULL, shutdown_listener, self);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&self->worker_threads[i], NULL, process_requests, self);
    }

    while (atomic_load(&self->running)) {
        int *active_socket = malloc(sizeof(int));
        if (!active_socket) {
            perror("ERROR: Failed to allocate memory for client socket");
            continue;
        }

        *active_socket = wait_for_client_connection(self->passive_socket);
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
        data->server = self;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_request, data) != 0) {
            perror("ERROR: Failed to create client thread");
            free(data);
            free(active_socket);
            continue;
        }
        append_thread_to_list(self, client_thread);
    }
    server_shutdown(self);
}

void server_shutdown(ServerContext *self) {
    sync_buffer_stop(&self->request_buffer);
    sync_buffer_stop(&self->response_buffer);

    pthread_join(self->response_thread, NULL);
    pthread_join(self->shutdown_thread, NULL);
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(self->worker_threads[i], NULL);
    }

    join_all_threads(self);

    sync_buff_destroy(&self->request_buffer);
    sync_buff_destroy(&self->response_buffer);
    close(self->passive_socket);
}
