#include "question.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void question_init(SyncQuestion* self, atomic_bool* lobby_running) {
    self->question = malloc(sizeof(Question));
    pthread_mutex_init(&self->mutex, NULL);
    pthread_cond_init(&self->read, NULL);
    pthread_cond_init(&self->write, NULL);
    self->lobby_running = lobby_running;
    self->num_players = 0;
}

void question_destroy(SyncQuestion* self) {
    free(self->question);
    self->question = NULL;
    pthread_mutex_destroy(&self->mutex);
    pthread_cond_destroy(&self->read);
    pthread_cond_destroy(&self->write);
}

void question_write(SyncQuestion* self, const Question* input) {
    atomic_store(self->question_ready, 0);
    pthread_mutex_lock(&self->mutex);
    if (atomic_load(self->lobby_running)) {
        memcpy(self->question, input, sizeof(Question));
        pthread_cond_signal(&self->read);
    }
    pthread_mutex_unlock(&self->mutex);
    atomic_store(self->question_ready, 1);
}

void question_read(SyncQuestion* self, Question* output) {
    pthread_mutex_lock(&self->mutex);
    while (atomic_load(self->question_ready) &&
           atomic_load(self->lobby_running)) {
        pthread_cond_wait(&self->read, &self->mutex);
    }
    if (atomic_load(self->lobby_running)) {
        memcpy(output, self->question, sizeof(Question));
    }
    pthread_mutex_unlock(&self->mutex);
}

void question_stop(SyncQuestion* self) {
    pthread_mutex_lock(&self->mutex);
    pthread_cond_broadcast(&self->write);
    pthread_cond_broadcast(&self->read);
    pthread_mutex_unlock(&self->mutex);
}
