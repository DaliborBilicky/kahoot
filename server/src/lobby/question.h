#ifndef QUESTION_H
#define QUESTION_H

#include <pthread.h>
#include <stdatomic.h>

#define MAX_CHOICES 4
#define MAX_TEXT_LEN 256

typedef struct Question {
    char text[MAX_TEXT_LEN];
    char choices[MAX_CHOICES][MAX_TEXT_LEN];
    int answer;
} Question;

typedef struct SyncQuestion {
    Question* question;
    pthread_mutex_t mutex;
    pthread_cond_t read;
    pthread_cond_t write;
    atomic_bool* lobby_running;
    int num_players;
    atomic_bool* question_ready;
} SyncQuestion;

void question_init(SyncQuestion* self, atomic_bool* lobby_running);
void question_destroy(SyncQuestion* self);
void question_write(SyncQuestion* self, const Question* input);
void question_read(SyncQuestion* self, Question* output);
void question_stop(SyncQuestion* self);

#endif  // QUESTION_H
