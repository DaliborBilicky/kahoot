#include "lobby.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../sockets/sockets.h"
#include "lobby_communication.h"

void extract_nickname(const char *message, char *output) {
    const char *nickname_start = strstr(message, "NICK:");
    if (nickname_start) {
        nickname_start += 5;
        output = strdup(nickname_start);
    }
}

void lobby_manager_init(LobbyManager *self) {
    sync_list_init(&self->lobby_list, sizeof(Lobby));
    atomic_store(&self->lobby_id_counter, 1);
}

int lobby_init(Lobby *self, int base_port, int lobby_id_counter) {
    atomic_store(&self->running, 1);
    int new_port = base_port + lobby_id_counter;

    self->id = new_port;
    self->port = new_port;
    self->current_players = 0;

    self->passive_socket = passive_socket_init(self->port);
    if (self->passive_socket < 0) {
        return -1;
    }

    self->thread_list_head = NULL;
    self->admin_thread = 0;
    sync_list_init(&self->players, sizeof(Player));
    question_init(&self->question, &self->running);

    return 0;
}

void lobby_shutdown(Lobby *self) {
    atomic_store(&self->running, 0);

    int dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (dummy_socket < 0) {
        perror("ERROR: Failed to create dummy socket");
        return;
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

    question_stop(&self->question);
    join_all_threads(&self->thread_list_head);
    if (self->admin_thread != 0) {
        pthread_join(self->admin_thread, NULL);
    }
    question_destroy(&self->question);
    close(self->passive_socket);
}

void lobby_shutdown_lambda(void *node, void *in, void *out, void *err) {
    Lobby *currentLobby = (Lobby *)((LinkedListNode *)node)->data;
    if (currentLobby) {
        lobby_shutdown(currentLobby);
    }
}

void shutdown_all_lobbies(SyncLinkedList *lobby_list) {
    sync_list_for_each(lobby_list, lobby_shutdown_lambda, NULL, NULL, NULL);
}

void lobby_manager_destroy(LobbyManager *self) {
    shutdown_all_lobbies(&self->lobby_list);
    sync_list_destroy(&self->lobby_list);
}

void lobby_run(Lobby *self) {
    if (self == NULL) {
        printf("Error: Lobby is null.\n");
        exit(1);
    }
    printf("Lobby running ID: %d\n", self->id);

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

        LobbyThreadData *data = malloc(sizeof(LobbyThreadData));
        if (!data) {
            perror("ERROR: Failed to allocate memory for RequestThreadData");
            close(*active_socket);
            free(active_socket);
            continue;
        }

        char message[MAX_REQUEST_LEN] = "";
        memset(message, 0, MAX_REQUEST_LEN);
        ssize_t bytes_read = read(*active_socket, message, MAX_REQUEST_LEN - 1);

        if (bytes_read <= 0) {
            printf("Client disconnected: %d\n", *active_socket);
            close(*active_socket);
            continue;
        }

        message[bytes_read] = '\0';
        data->lobby = self;
        data->active_socket = active_socket;
        if (strncmp(message, "A JOIN_LOBBY", 12) == 0) {
            printf("Super user connected: %d\n", *active_socket);

            if (pthread_create(&self->admin_thread, NULL, handle_admin, data) !=
                0) {
                perror("ERROR: Failed to create super user thread");
                close(*active_socket);
                free(active_socket);
                free(data);
            }
        } else if (strncmp(message, "P JOIN_LOBBY", 12) == 0) {
            printf("Player connected: %d\n", *active_socket);

            pthread_t player_thread;
            if (pthread_create(&player_thread, NULL, handle_player, data) !=
                0) {
                perror("ERROR: Failed to create client thread");
                close(*active_socket);
                free(active_socket);
                free(data);
            } else {
                Player player;
                player.nick = "meno \0";
                player.score = 0;
                append_thread_to_list(&self->thread_list_head, player_thread);
                sync_list_add(&self->players, &player);
                self->current_players++;
            }
        } else {
            printf("Invalid request from client: %d\n", *active_socket);
            close(*active_socket);
            free(active_socket);
            free(data);
        }
    }
    lobby_shutdown(self);

    printf("Lobby %d process ended\n", self->id);
    exit(0);
}

int lobby_manager_create_lobby(LobbyManager *self, int base_port) {
    Lobby new_lobby = {0};
    if (lobby_init(&new_lobby, base_port,
                   atomic_fetch_add(&self->lobby_id_counter, 1)) < 0) {
        fprintf(stderr, "ERROR: Failed to create lobby\n");
        return -1;
    }
    sync_list_add(&self->lobby_list, &new_lobby);

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "ERROR: Failed to fork process\n");
        close(new_lobby.passive_socket);
        return -1;
    }
    if (pid == 0) {
        lobby_run(sync_list_get_tail_data(&self->lobby_list));
    } else {
        printf("Created lobby %d, with process ID: %d\n", new_lobby.id, pid);
    }

    return new_lobby.id;
}
