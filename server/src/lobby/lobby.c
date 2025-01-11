#include "lobby.h"

#include <stdio.h>
#include <unistd.h>

#include "../sockets/sockets.h"

void lobby_manager_init(LobbyManager *self) {
    sync_list_init(&self->lobby_list, sizeof(Lobby));
}

void lobby_manager_destroy(LobbyManager *self) {
    sync_list_destroy(&self->lobby_list);
}

int lobby_init(Lobby *self, int base_port, int lobby_id_counter) {
    atomic_store(&self->running, 1);
    int new_port = base_port + lobby_id_counter;

    self->id = new_port;
    self->port = new_port;
    self->current_players = 0;
    self->max_players = 100;

    self->passive_socket = passive_socket_init(self->port);
    if (self->passive_socket < 0) {
        return -1;
    }

    return 0;
}

void lobby_shutdown(Lobby *self) { atomic_store(&self->running, 0); }

void find_lobby_by_id(void *node, void *in, void *out, void *err) {
    Lobby *currentProcess = (Lobby *)((LinkedListNode *)node)->data;
    int *targetId = (int *)in;

    if (currentProcess->id == *targetId) {
        *(Lobby **)out = currentProcess;
    }
}

Lobby *get_lobby_by_id(SyncLinkedList *lobby_list, int lobby_id) {
    Lobby *lobby = NULL;
    sync_list_for_each(lobby_list, find_lobby_by_id, &lobby_id, &lobby, NULL);
    return lobby;
}

void lobby_run(Lobby *self) {
    if (self == NULL) {
        printf("Error: Lobby is null.\n");
        exit(1);
    }
    printf("Lobby ID: %d\n", self->id);

    pthread_create(&self->super_user_thread, NULL, handle_super_user, self);

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

        PlayerThreadData *player = malloc(sizeof(PlayerThreadData));
        if (!player) {
            perror("ERROR: Failed to allocate memory for RequestThreadData");
            close(*active_socket);
            free(active_socket);
            continue;
        }

        player->active_socket = active_socket;
        player->lobby = self;
        player->score = 0;
        strncpy(player->nickname, "Player1", MAX_NICKNAME_LEN);
        player->nickname[MAX_NICKNAME_LEN - 1] = '\0';

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_request, player) != 0) {
            perror("ERROR: Failed to create client thread");
            free(player);
            free(active_socket);
            continue;
        }
        // add to list of threads
    }
    lobby_shutdown(self);

    printf("Lobby %d process ended\n", self->id);
    exit(0);
}

int lobby_manager_create_lobby(LobbyManager *self, int base_port) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "ERROR: Failed to fork process\n");
        return -1;
    }

    Lobby new_lobby;
    if (lobby_init(&new_lobby, base_port, self->lobby_id_counter) < 0) {
        fprintf(stderr, "ERROR: Failed to create lobby\n");
        return -1;
    }

    sync_list_add(&self->lobby_list, &new_lobby);

    if (pid == 0) {
        lobby_run(sync_list_get_tail_data(&self->lobby_list));
    } else {
        printf("Created lobby %d, with process ID: %d\n", new_lobby.id, pid);
    }

    return new_lobby.id;
}
