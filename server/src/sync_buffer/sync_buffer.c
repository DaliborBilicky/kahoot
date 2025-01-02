#include "sync_buffer.h"

#include "buffer.h"

void sync_buff_init(SynchronizedBuffer* self, size_t capacity,
                    size_t data_size) {
    buffer_init(&self->buffer, capacity, data_size);
    pthread_mutex_init(&self->mutex, NULL);
    pthread_cond_init(&self->consume, NULL);
    pthread_cond_init(&self->produce, NULL);
}

void sync_buff_destroy(SynchronizedBuffer* self) {
    buffer_destroy(&self->buffer);
    pthread_mutex_destroy(&self->mutex);
    pthread_cond_destroy(&self->produce);
    pthread_cond_destroy(&self->consume);
}

void sync_buff_push(SynchronizedBuffer* self, const void* input) {
    pthread_mutex_lock(&self->mutex);
    while (self->buffer.capacity == self->buffer.size) {
        pthread_cond_wait(&self->produce, &self->mutex);
    }
    buffer_push(&self->buffer, input);
    pthread_cond_signal(&self->consume);
    pthread_mutex_unlock(&self->mutex);
}

void sync_buff_pop(SynchronizedBuffer* self, void* output) {
    pthread_mutex_lock(&self->mutex);
    while (self->buffer.size == 0) {
        pthread_cond_wait(&self->consume, &self->mutex);
    }
    buffer_pop(&self->buffer, output);
    pthread_cond_signal(&self->produce);
    pthread_mutex_unlock(&self->mutex);
}
