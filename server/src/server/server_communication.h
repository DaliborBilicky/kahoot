#ifndef SERVER_COMMUNICATION_H
#define SERVER_COMMUNICATION_H

#include "server.h"

typedef struct ClientMessage {
    int active_socket;
    char message[MAX_REQUEST_LEN];
} ClientMessage;

typedef struct RequestThreadData {
    ServerContext *context;
    int *active_socket;
} RequestThreadData;

void *handle_request(void *arg);
void *process_requests(void *arg);
void *send_responses(void *arg);

#endif  // SERVER_COMMUNICATION_H
