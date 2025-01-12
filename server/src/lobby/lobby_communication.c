#include "lobby_communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void process_player(void* item, void* in, void* out, void* err) {
    Player* player = (Player*)item;
    int* best_score = (int*)in;
    Player** best_player = (Player**)out;

    if (player->score > *best_score) {
        *best_score = player->score;
        *best_player = player;
    }
}

void find_best_score(SyncLinkedList* self, Player* output) {
    pthread_mutex_lock(&self->muttex);

    Player* best_player = NULL;
    int best_score = -1;

    sync_list_for_each(self, process_player, &best_score, &best_player, NULL);

    pthread_mutex_unlock(&self->muttex);

    output = best_player;
}

void* handle_admin(void* arg) {
    LobbyThreadData* data = (LobbyThreadData*)arg;
    Lobby* lobby = data->lobby;
    int active_socket = *(data->active_socket);

    free(data->active_socket);
    free(data);

    printf("IN admin thread\n");

    send(active_socket, "LOBBY_JOINED", 12, 0);

    char buffer[1024];
    while (atomic_load(&lobby->running)) {
        int bytes_received = recv(active_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Admin disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Admin command received: %s\n", buffer);

        if (strncmp(buffer, "START_GAME", 10) == 0) {
            printf("Starting game in lobby %d\n", lobby->id);
        } else if (strncmp(buffer, "LOAD_QUESTION", 14) == 0) {
            char* message_copy = strdup(buffer);
            char* saveptr;

            strtok_r(message_copy, ":", &saveptr);
            char* question_text = strtok_r(NULL, ";", &saveptr);
            char* choices = strtok_r(NULL, ";", &saveptr);
            int correct_answer = atoi(strtok_r(NULL, ";", &saveptr));
            Question question;
            strncpy(question.text, question_text, MAX_TEXT_LEN - 1);
            question.text[MAX_TEXT_LEN - 1] = '\0';
            question.answer = correct_answer;
            int i = 0;
            char* answer = strtok_r(choices, "|", &saveptr);
            while (answer != NULL && i < MAX_CHOICES) {
                strncpy(question.choices[i], answer, MAX_TEXT_LEN - 1);
                answer = strtok_r(NULL, "|", &saveptr);
                i++;
            }

            free(message_copy);
            send(active_socket, "QUESTION_RECIEVED", 17, 0);
        } else if (strncmp(buffer, "GET_WINNER", 11) == 0) {
            Player player;
            find_best_score(&lobby->players, &player);
            char message[MAX_REQUEST_LEN];
            snprintf(message, MAX_REQUEST_LEN, "WINNER NICK:%s, SCORE:%d\n",
                     player.nick, player.score);

            send(active_socket, message, 13, 0);
        } else if (strncmp(buffer, "SHUTDOWN", 8) == 0) {
            printf("Shutting down lobby %d\n", lobby->id);
            atomic_store(&lobby->running, 0);
            send(active_socket, "LOBBY_SHUTDOWN", 15, 0);
            break;
        } else {
            const char* error_response = "ERROR:Invalid request";
            send(active_socket, error_response, strlen(error_response), 0);
        }
    }

    close(active_socket);
    return NULL;
}

void* handle_player(void* arg) {
    LobbyThreadData* data = (LobbyThreadData*)arg;
    Lobby* lobby = data->lobby;
    int active_socket = *(data->active_socket);

    free(data->active_socket);
    free(data);

    printf("IN player thread\n");

    char buffer[1024];
    while (atomic_load(&lobby->running)) {
        if (atomic_load(lobby->question.question_ready)) {
            Question question = {0};
            question_read(&lobby->question, &question);
            char formatted_question[MAX_REQUEST_LEN];

            snprintf(formatted_question, MAX_REQUEST_LEN,
                     "QUESTION:%s:%s,%s,%s,%s", question.text,
                     question.choices[0], question.choices[1],
                     question.choices[2], question.choices[3]);

            send(active_socket, formatted_question, strlen(formatted_question),
                 0);
            ssize_t received_len =
                recv(active_socket, buffer, sizeof(buffer) - 1, 0);
            if (received_len < 0) {
                printf("Failed to receive answer from player\n");
                break;
            }

            buffer[received_len] = '\0';

            if (strncmp(buffer, "ANSWER_SUBMIT ANSWER:", 22) == 0) {
                int player_answer = atoi(buffer + 22);

                if (player_answer == question.answer) {
                    const char* response = "RESULT:1";
                    send(active_socket, response, strlen(response), 0);
                } else {
                    const char* response = "RESULT:0";
                    send(active_socket, response, strlen(response), 0);
                }
            } else {
                const char* error_response = "ERROR:Invalid answer format";
                send(active_socket, error_response, strlen(error_response), 0);
            }
        }
    }

    close(active_socket);
    return NULL;
}
