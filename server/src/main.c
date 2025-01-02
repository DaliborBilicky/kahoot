#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "sync_buffer/sync_buffer.h"

#define BUFFER_CAPACITY 5
#define NUM_ITEMS 10

typedef struct {
    SynchronizedBuffer* buffer;
    int id;
} ThreadArg;

void* producer(void* arg) {
    ThreadArg* threadArg = (ThreadArg*)arg;
    SynchronizedBuffer* buffer = threadArg->buffer;
    int producer_id = threadArg->id;

    for (int i = 0; i < NUM_ITEMS; i++) {
        printf("Producer %d: Producing item %d\n", producer_id, i);
        sync_buff_push(buffer, &i);
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    ThreadArg* threadArg = (ThreadArg*)arg;
    SynchronizedBuffer* buffer = threadArg->buffer;
    int consumer_id = threadArg->id;

    for (int i = 0; i < NUM_ITEMS; i++) {
        int item;
        sync_buff_pop(buffer, &item);
        printf("Consumer %d: Consumed item %d\n", consumer_id, item);
        sleep(1);
    }
    return NULL;
}

int main() {
    SynchronizedBuffer buffer;
    sync_buff_init(&buffer, BUFFER_CAPACITY, sizeof(int));

    pthread_t producer_thread, consumer_thread;

    ThreadArg producer_arg = {&buffer, 1};
    ThreadArg consumer_arg = {&buffer, 1};

    pthread_create(&producer_thread, NULL, producer, &producer_arg);
    pthread_create(&consumer_thread, NULL, consumer, &consumer_arg);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sync_buff_destroy(&buffer);

    return 0;
}
