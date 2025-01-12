#include "server_communication.h"

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
    char message[MAX_REQUEST_LEN] = "";
    int authenticated = 0;

    client_message.active_socket = active_socket;

    while (atomic_load(&context->running)) {
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
            if (strncmp(message, context->password,
                        strlen(context->password)) == 0) {
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
            sync_buff_push(&context->request_buffer, &client_message);
        }
    }
    close(active_socket);
    return NULL;
}

void *process_requests(void *arg) {
    ServerContext *context = (ServerContext *)arg;
    ClientMessage client_message = {0};

    while (atomic_load(&context->running)) {
        sync_buff_pop(&context->request_buffer, &client_message);

        // -----------------------CREATE LOBBY--------------------------
        if (strncmp(client_message.message, "CREATE_LOBBY", 12) == 0) {
            int lobby_id = create_lobby(context, context->port);
            Lobby *lobby = get_lobby_by_id(context, lobby_id);

            if (lobby != NULL) {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "CREATE_SUCCESS LOBBY_CREATED ID:%d PORT:%d "
                         "CURRENT_PLAYERS:%d MAX_PLAYERS:%d",
                         lobby->id, lobby->port, lobby->current_players,
                         lobby->max_players);
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
            } else {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "ERROR: Could not create lobby.");
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
            }

            // -----------------------JOIN LOBBY--------------------------
        } else if (strncmp(client_message.message, "JOIN_LOBBY", 10) == 0) {
            int lobby_id = atoi(client_message.message + 11);
            Lobby *lobby = get_lobby_by_id(context, lobby_id);

            if (lobby != NULL && lobby->current_players < lobby->max_players) {
                lobby->clients[lobby->current_players] =
                    client_message.active_socket;
                lobby->current_players++;
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "JOIN_SUCCESS LOBBY_ID:%d CURRENT_PLAYERS:%d "
                         "MAX_PLAYERS:%d",
                         lobby_id, lobby->current_players, lobby->max_players);
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';

            } else if (lobby == NULL) {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "ERROR: Lobby not found.");
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
            } else if (lobby->current_players >= lobby->max_players) {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "ERROR: Lobby is full.");
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
            }
        }

        // -----------------------SEND UPDATES TO GUI--------------------------
        else if (strncmp(client_message.message, "HOST_UPDATE", 11) == 0) {
            int lobby_id = atoi(client_message.message + 12);
            Lobby *lobby = get_lobby_by_id(context, lobby_id);

            if (lobby != NULL) {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "LOBBY_STATUS ID:%d CURRENT_PLAYERS:%d MAX_PLAYERS:%d",
                         lobby_id, lobby->current_players, lobby->max_players);
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';

            } else {
                snprintf(client_message.message, MAX_RESPONSE_LEN,
                         "ERROR: Lobby not found.");
                client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
            }

            // -----------------------GET STATUS--------------------------
        } else if (strncmp(client_message.message, "GET_STATUS", 10) == 0) {
            snprintf(client_message.message, MAX_RESPONSE_LEN,
                     "SERVER_STATUS STATUS:Running");
            client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
            // -----------------------SEND QUESTION TO LOBBY
        } else if (strncmp(client_message.message, "SEND_QUESTION", 13) == 0) {
            int lobby_id = atoi(client_message.message + 14);
            printf("SERVER: Received SEND_QUESTION request for lobby %d\n",
                   lobby_id);
            const char *question = "What is the capital of France?";
            send_question_to_lobby(context, lobby_id, question);
            printf("SERVER: Sent question to lobby %d\n", lobby_id);
        }

        // -----------------------PROCESS ANSWERS--------------------------
        else if (strncmp(client_message.message, "ANSWER:", 7) == 0) {
            char *answer = client_message.message + 7;
            printf("Received answer from client %d: %s\n",
                   client_message.active_socket, answer);

            const char *correct_answer = "Paris";
            if (strcmp(answer, correct_answer) == 0) {
                const char *response = "Correct answer!";
                send(client_message.active_socket, response, strlen(response),
                     0);
            } else {
                const char *response = "Incorrect answer!";
                send(client_message.active_socket, response, strlen(response),
                     0);
            }
        }
        //------------------------LOAD QUESTIONS--------------------
        else if (strncmp(client_message.message, "LOAD_QUESTION", 12) == 0) {
            char *message_copy = strdup(client_message.message);
            char *saveptr;
            char *token = strtok_r(message_copy, ";", &saveptr);

            token = strtok_r(NULL, ";", &saveptr);
            int lobby_id = atoi(token);

            char *question_text = strtok_r(NULL, ";", &saveptr);
            char *answers = strtok_r(NULL, ";", &saveptr);
            int correct_answer = atoi(strtok_r(NULL, ";", &saveptr));

            Lobby *lobby = get_lobby_by_id(context, lobby_id);
            if (lobby) {
                Question *new_questions =
                    realloc(lobby->questions,
                            (lobby->num_questions + 1) * sizeof(Question));
                if (new_questions) {
                    lobby->questions = new_questions;
                    Question *current = &lobby->questions[lobby->num_questions];

                    strncpy(current->question, question_text,
                            MAX_QUESTION_LENGTH - 1);
                    printf("DEBUG: Adding question %d: %s\n",
                           lobby->num_questions + 1, question_text);

                    char *ans_saveptr;
                    char *ans = strtok_r(answers, "|", &ans_saveptr);
                    int i = 0;
                    while (ans && i < MAX_ANSWERS) {
                        strncpy(current->answers[i], ans,
                                MAX_ANSWER_LENGTH - 1);
                        printf("DEBUG: Answer %d: %s\n", i + 1, ans);
                        ans = strtok_r(NULL, "|", &ans_saveptr);
                        i++;
                    }

                    current->correct_answer = correct_answer;
                    printf("DEBUG: Correct answer: %d\n", correct_answer);
                    lobby->num_questions++;

                    printf("DEBUG: Lobby %d now has %d questions:\n", lobby->id,
                           lobby->num_questions);
                    for (int j = 0; j < lobby->num_questions; j++) {
                        printf("DEBUG: Question %d: %s\n", j + 1,
                               lobby->questions[j].question);
                        for (int k = 0; k < MAX_ANSWERS; k++) {
                            printf("DEBUG: Answer %d: %s\n", k + 1,
                                   lobby->questions[j].answers[k]);
                        }
                        printf("DEBUG: Correct answer: %d\n",
                               lobby->questions[j].correct_answer);
                    }

                    snprintf(client_message.message, MAX_RESPONSE_LEN,
                             "QUESTION_LOADED:%d", lobby->num_questions);
                }
            }
            free(message_copy);
        }
        // -----------------------INVALID REQUEST--------------------------
        else {
            snprintf(client_message.message, MAX_RESPONSE_LEN,
                     "ERROR INVALID_REQUEST:Unknown-command");
            client_message.message[MAX_RESPONSE_LEN - 1] = '\0';
        }

        sync_buff_push(&context->response_buffer, &client_message);
    }
    return NULL;
}

void *send_responses(void *arg) {
    ServerContext *context = (ServerContext *)arg;
    ClientMessage client_message = {0};

    while (atomic_load(&context->running)) {
        sync_buff_pop(&context->response_buffer, &client_message);

        send(client_message.active_socket, client_message.message,
             strlen(client_message.message), 0);
        printf("Sent response to client %d: %s\n", client_message.active_socket,
               client_message.message);
    }
    return NULL;
}
