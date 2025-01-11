#include "sync_buffer.h"

#include <stdio.h>

#include "buffer.h"

void sync_buff_init(SynchronizedBuffer* self, size_t capacity, size_t data_size,
                    atomic_bool* server_running) {
    buffer_init(&self->buffer, capacity, data_size);
    pthread_mutex_init(&self->mutex, NULL);
    pthread_cond_init(&self->consume, NULL);
    pthread_cond_init(&self->produce, NULL);
    self->server_running = server_running;
}

void sync_buff_destroy(SynchronizedBuffer* self) {
    buffer_destroy(&self->buffer);
    pthread_mutex_destroy(&self->mutex);
    pthread_cond_destroy(&self->produce);
    pthread_cond_destroy(&self->consume);
}

void sync_buff_push(SynchronizedBuffer* self, const void* input) {
    pthread_mutex_lock(&self->mutex);
    while (self->buffer.capacity == self->buffer.size &&
           atomic_load(self->server_running)) {
        pthread_cond_wait(&self->produce, &self->mutex);
    }
    if (atomic_load(self->server_running)) {
        buffer_push(&self->buffer, input);
        pthread_cond_signal(&self->consume);
    }
    pthread_mutex_unlock(&self->mutex);
}

void sync_buff_pop(SynchronizedBuffer* self, void* output) {
    pthread_mutex_lock(&self->mutex);
    while (self->buffer.size == 0 && atomic_load(self->server_running)) {
        pthread_cond_wait(&self->consume, &self->mutex);
    }
    if (atomic_load(self->server_running)) {
        buffer_pop(&self->buffer, output);
        pthread_cond_signal(&self->produce);
    }
    pthread_mutex_unlock(&self->mutex);
}

void sync_buff_stop(SynchronizedBuffer* self) {
    pthread_mutex_lock(&self->mutex);
    pthread_cond_broadcast(&self->consume);
    pthread_cond_broadcast(&self->produce);
    pthread_mutex_unlock(&self->mutex);
}
