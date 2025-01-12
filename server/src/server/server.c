#include "server.h"

#include <stdio.h>
#include <stdlib.h>
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

    context->lobbies = NULL;

    return 0;
}

void handle_lobby(int lobby_id, ServerContext *context) {
    Lobby *lobby = get_lobby_by_id(context, lobby_id);
    if (lobby == NULL) {
        printf("Error: Lobby ID %d not found\n", lobby_id);
        exit(1);
    }

    printf("Lobby ID: %d | Current Players: %d\n", lobby->id,
           lobby->current_players);

    while (1) {
        sleep(1);
    }
}

void send_question_to_lobby(ServerContext *context, int lobby_id,
                            const char *question) {
    if (!context) {
        printf("Invalid context\n");
        return;
    }

    Lobby *lobby = get_lobby_by_id(context, lobby_id);
    if (lobby == NULL) {
        printf("Lobby with ID %d not found\n", lobby_id);
        return;
    }

    printf("DEBUG: Lobby %d has %d questions\n", lobby_id,
           lobby->num_questions);

    if (lobby->num_questions == 0 || !lobby->questions) {
        printf("No questions loaded for lobby %d\n", lobby_id);
        return;
    }

    if (lobby->current_question >= lobby->num_questions) {
        printf("All questions have been sent\n");
        return;
    }

    Question *current = &lobby->questions[lobby->current_question];
    char formatted_question[MAX_REQUEST_LEN];

    snprintf(formatted_question, MAX_REQUEST_LEN, "QUESTION:%s:%s,%s,%s,%s",
             current->question, current->answers[0], current->answers[1],
             current->answers[2], current->answers[3]);

    printf("Sending question %d to lobby %d\n", lobby->current_question + 1,
           lobby_id);

    for (int i = 0; i < lobby->current_players; i++) {
        if (lobby->clients[i] <= 0) {
            continue;
        }

        if (send(lobby->clients[i], formatted_question,
                 strlen(formatted_question), 0) < 0) {
            printf("Failed to send question to client %d\n", lobby->clients[i]);
        } else {
            printf("Sent question to client %d: %s\n", lobby->clients[i],
                   formatted_question);
        }
    }

    lobby->current_question++;
}

int create_lobby(ServerContext *context, int base_port) {
    static int lobby_id_counter = 1;
    int new_port = base_port + lobby_id_counter;

    Lobby *new_lobby = (Lobby *)malloc(sizeof(Lobby));
    if (!new_lobby) {
        return -1;
    }

    new_lobby->id = lobby_id_counter++;
    new_lobby->port = new_port;
    new_lobby->current_players = 0;
    new_lobby->max_players = MAX_PLAYERS;
    new_lobby->next = context->lobbies;
    context->lobbies = new_lobby;

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        free(new_lobby);
        return -1;
    }

    if (pid == 0) {
        handle_lobby(new_lobby->id, context);
    } else {
        printf("Created lobby %d, with process ID: %d\n", new_lobby->id, pid);
    }

    return new_lobby->id;
}

Lobby *get_lobby_by_id(ServerContext *context, int lobby_id) {
    Lobby *lobby = context->lobbies;
    while (lobby) {
        if (lobby->id == lobby_id) {
            return lobby;
        }
        lobby = lobby->next;
    }
    return NULL;
}

void *shutdown_listener(void *arg) {
    ServerContext *context = (ServerContext *)arg;
    int command = 0;

    printf("Press CTRL+D to shutdown the server.\n");

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

    Lobby *lobby = context->lobbies;
    while (lobby) {
        Lobby *temp = lobby;
        lobby = lobby->next;
        free(temp);
    }

    sync_buff_destroy(&context->request_buffer);
    sync_buff_destroy(&context->response_buffer);

    close(context->passive_socket);
}
