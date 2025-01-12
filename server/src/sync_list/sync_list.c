#include "sync_list.h"

void sync_list_init(SyncLinkedList *self, size_t data_size) {
    linked_list_init(&self->list, data_size);
    pthread_mutex_init(&self->muttex, NULL);
}

void sync_list_destroy(SyncLinkedList *self) {
    pthread_mutex_lock(&self->muttex);
    linked_list_destroy(&self->list);
    pthread_mutex_unlock(&self->muttex);
    pthread_mutex_destroy(&self->muttex);
}

size_t sync_list_get_size(SyncLinkedList *self) {
    pthread_mutex_lock(&self->muttex);
    size_t size = linked_list_get_size(&self->list);
    pthread_mutex_unlock(&self->muttex);
    return size;
}

void sync_list_add(SyncLinkedList *self, void *data) {
    pthread_mutex_lock(&self->muttex);
    linked_list_add(&self->list, data);
    pthread_mutex_unlock(&self->muttex);
}

void sync_list_for_each(SyncLinkedList *self,
                        void (*process_item)(void *, void *, void *, void *),
                        void *in, void *out, void *err) {
    pthread_mutex_lock(&self->muttex);
    linked_list_for_each(&self->list, process_item, in, out, err);
    pthread_mutex_unlock(&self->muttex);
}

void *sync_list_get_tail_data(SyncLinkedList *self) {
    pthread_mutex_lock(&self->muttex);
    void *data = linked_list_get_tail_data(&self->list);
    pthread_mutex_unlock(&self->muttex);
    return data;
}
