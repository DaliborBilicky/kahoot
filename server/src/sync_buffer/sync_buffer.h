#ifndef SYNC_BUFFER_H
#define SYNC_BUFFER_H

#include <pthread.h>

#include "buffer.h"

typedef struct SynchronizedBuffer {
    Buffer buffer;
    pthread_mutex_t mutex;
    pthread_cond_t consume;
    pthread_cond_t produce;
} SynchronizedBuffer;

void sync_buff_init(SynchronizedBuffer* self, size_t capacity,
                    size_t data_size);
void sync_buff_destroy(SynchronizedBuffer* self);
void sync_buff_push(SynchronizedBuffer* self, const void* input);
void sync_buff_pop(SynchronizedBuffer* self, void* output);

#endif  // SYNC_BUFFER_H
