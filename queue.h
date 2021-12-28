#ifndef QUEUE_H__
#define QUEUE_H__

#include <stddef.h>

#include <pthread.h>

struct queue_link {
    struct queue_link *next, *prev;
};

struct queue_head {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    struct queue_link queue;
};

#define queue_entry(ptr, type, member) \
    (type *)((char *)(ptr) - offsetof(type, member))
#define dequeue_entry(head, type, member) \
    queue_entry(dequeue(head), type, member)

void queue_head_init(struct queue_head *head);
void enqueue(struct queue_link *new, struct queue_head *head);
struct queue_link *dequeue(struct queue_head *head);
int queue_empty(const struct queue_head *head);
void queue_head_destroy(struct queue_head *head);

#endif
