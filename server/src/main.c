#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "server/server.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s <port> <password>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    const char *password = argv[2];

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
