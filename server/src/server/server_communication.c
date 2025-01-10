#include "server_communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *handle_request(void *arg) {
    RequestThreadData *data = (RequestThreadData *)arg;
    ServerContext *server = data->server;
    int active_socket = *(data->active_socket);

    free(data->active_socket);
    free(data);

    ClientMessage client_message;
    char message[MAX_REQUEST_LEN] = "";
    int authenticated = 0;

    client_message.active_socket = active_socket;

    while (atomic_load(&server->running)) {
        memset(message, 0, MAX_REQUEST_LEN);
        ssize_t bytes_read = read(active_socket, message, MAX_REQUEST_LEN - 1);

        if (bytes_read <= 0) {
            printf("Client disconnected: %d\n", active_socket);
            close(active_socket);
            return NULL;
        }

        message[bytes_read] = '\0';
        printf("Request from client %d: %s\n", active_socket, message);

        if (!authenticated) {
            if (strncmp(message, server->password,
                        strlen(server->password)) == 0) {
                authenticated = 1;
                const char *auth_success = "AUTH_SUCCESS";
                send(active_socket, auth_success, strlen(auth_success), 0);
                printf("CLIENT_AUTHENTICATED\n");
            } else {
                const char *auth_fail = "Invalid password";
                send(active_socket, auth_fail, strlen(auth_fail), 0);
                close(active_socket);
                return NULL;
            }
        } else {
            strncpy(client_message.message, message, MAX_REQUEST_LEN - 1);
            sync_buff_push(&server->request_buffer, &client_message);
        }
    }
    close(active_socket);
    return NULL;
}

void *process_requests(void *arg) {
    ServerContext *server = (ServerContext *)arg;
    ClientMessage client_message = {0};

    while (atomic_load(&server->running)) {
        sync_buff_pop(&server->request_buffer, &client_message);

        if (strncmp(client_message.message, "CREATE_LOBBY", 12) == 0) {
            int port = 9000;
            int lobby_id = port + 12340000;

            snprintf(client_message.message, MAX_RESPONSE_LEN,
                     "LOBBY_CREATED ID:%d PORT:%d", lobby_id, port);
            client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
        } else if (strncmp(client_message.message, "GET_STATUS", 10) == 0) {
            snprintf(client_message.message, MAX_RESPONSE_LEN,
                     "SERVER_STATUS STATUS:Running");
            client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
        } else {
            snprintf(client_message.message, MAX_RESPONSE_LEN,
                     "ERROR INVALID_REQUEST:Unknown-command");
            client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
        }

        sync_buff_push(&server->response_buffer, &client_message);
    }
    return NULL;
}

void *send_responses(void *arg) {
    ServerContext *server = (ServerContext *)arg;
    ClientMessage client_message = {0};

    while (atomic_load(&server->running)) {
        sync_buff_pop(&server->response_buffer, &client_message);

        send(client_message.active_socket, client_message.message,
             strlen(client_message.message), 0);
        printf("Sent response to client %d: %s\n", client_message.active_socket,
               client_message.message);
    }
    return NULL;
}
