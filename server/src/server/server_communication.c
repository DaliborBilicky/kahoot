#include "server_communication.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *handle_request(void *arg) {
    RequestThreadData *data = (RequestThreadData *)arg;
    ServerContext *context = data->context;
    int active_socket = *(data->active_socket);

    free(data->active_socket);
    free(data);

    ClientMessage client_message;
    char buffer[MAX_REQUEST_LEN];
    int authenticated = 0;

    client_message.active_socket = active_socket;

    while (1) {
        memset(buffer, 0, MAX_REQUEST_LEN);
        ssize_t bytes_read = read(active_socket, buffer, MAX_REQUEST_LEN - 1);

        if (bytes_read <= 0) {
            printf("Client disconnected: %d\n", active_socket);
            close(active_socket);
            return NULL;
        }

        buffer[bytes_read] = '\0';
        printf("Request from client %d: %s\n", active_socket, buffer);

        if (!authenticated) {
            if (strncmp(buffer, context->password, strlen(context->password)) ==
                0) {
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
            strncpy(client_message.message, buffer, MAX_REQUEST_LEN - 1);
            sync_buff_push(&context->request_buffer, &client_message);
        }
    }
}

void *process_requests(void *arg) {
    ServerContext *context = (ServerContext *)arg;
    ClientMessage client_message;

    while (1) {
        sync_buff_pop(&context->request_buffer, &client_message);

        if (strncmp(client_message.message, "CREATE_LOBBY", 12) == 0) {
            int port = 9000;
            int lobby_id = create_lobby(context, port);
            Lobby *lobby = get_lobby_by_id(context, lobby_id);

            if (lobby != NULL) {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                 "LOBBY_CREATED ID:%d PORT:%d CURRENT_PLAYERS:%d MAX_PLAYERS:%d",
                 lobby->id, lobby->port, lobby->current_players, lobby->max_players);
            } else {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                 "ERROR: Could not retrieve lobby details.");
            }
        
        } else {
            snprintf(client_message.message, MAX_RESPONSE_LEN,
                     "ERROR INVALID_REQUEST: Unknown-command");
        }

        sync_buff_push(&context->response_buffer, &client_message);
    }
}


void *send_responses(void *arg) {
    ServerContext *context = (ServerContext *)arg;
    ClientMessage client_message;

    while (1) {
        sync_buff_pop(&context->response_buffer, &client_message);

        send(client_message.active_socket, client_message.message,
             strlen(client_message.message), 0);
        printf("Sent response to client %d: %s\n", client_message.active_socket,
               client_message.message);
    }
}
