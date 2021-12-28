#include "queue.h"

#include <stdlib.h>

void
queue_head_init(struct queue_head *head)
{
    pthread_mutex_init(&head->mtx, NULL);
    pthread_cond_init(&head->cond, NULL);
    head->queue.prev = head->queue.next = &head->queue;
}

void
queue_head_destroy(struct queue_head *head)
{
    pthread_mutex_destroy(&head->mtx);
    pthread_cond_destroy(&head->cond);
}

static void
queue_add(struct queue_link *new,
          struct queue_link *prev,
          struct queue_link *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void
enqueue(struct queue_link *new, struct queue_head *head)
{
    pthread_mutex_lock(&head->mtx);
    queue_add(new, head->queue.prev, &head->queue);
    pthread_mutex_unlock(&head->mtx);
    pthread_cond_signal(&head->cond);
}

static void
queue_del(struct queue_link *prev, struct queue_link *next)
{
    next->prev = prev;
    prev->next = next;
}

struct queue_link *
dequeue(struct queue_head *head)
{
    struct queue_link *first;

    pthread_mutex_lock(&head->mtx);

    while (queue_empty(head))
        pthread_cond_wait(&head->cond, &head->mtx);

    first = head->queue.next;
    queue_del(&head->queue, head->queue.next->next);
    first->prev = first->next = NULL;
    pthread_mutex_unlock(&head->mtx);
    return first;
}

int
queue_empty(const struct queue_head *head)
{
    return head->queue.next == &head->queue;
}
