#include "lobby.h"

#include <stdio.h>
#include <unistd.h>

void lobby_manager_init(LobbyManager *self) {
    sync_list_init(&self->lobby_list, sizeof(Lobby));
}

void lobby_destroy(LobbyManager *self) { sync_list_destroy(&self->lobby_list); }

void find_lobby_by_id(void *node, void *in, void *out, void *err) {
    Lobby *currentProcess = (Lobby *)((LinkedListNode *)node)->data;
    int *targetId = (int *)in;

    if (currentProcess->id == *targetId) {
        *(Lobby **)out = currentProcess;
    }
}

Lobby *get_lobby_by_id(LobbyManager *self, int lobby_id) {
    Lobby *lobby = NULL;
    sync_list_for_each(&self->lobby_list, find_lobby_by_id, &lobby_id, &lobby,
                       NULL);
    return lobby;
}

void handle_lobby(LobbyManager *self, int lobby_id) {
    Lobby *lobby = get_lobby_by_id(self, lobby_id);
    if (lobby == NULL) {
        printf("Error: Lobby ID %d not found\n", lobby_id);
        exit(1);
    }

    printf("Lobby ID: %d | Current Players: %d\n", lobby->id,
           lobby->current_players);

    while (1) {
        sleep(1);
    }

    printf("Lobby %d process ended\n", lobby_id);
    exit(0);
}

int create_lobby(LobbyManager *self, int base_port) {
    static int lobby_id_counter = 1;
    int new_port = base_port + lobby_id_counter;

    Lobby new_lobby;
    new_lobby.id = lobby_id_counter++;
    new_lobby.port = new_port;
    new_lobby.current_players = 0;
    new_lobby.max_players = 100;

    Lobby new_lobby_process;

    sync_list_add(&self->lobby_list, &new_lobby_process);

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {
        handle_lobby(self, new_lobby.id);
    } else {
        printf("Created lobby %d, with process ID: %d\n", new_lobby.id, pid);
    }

    return new_lobby.id;
}
