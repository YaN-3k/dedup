#include <pthread.h>
#include <stdlib.h>

#include "queue.h"
#include "util.h"

struct queue_t {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    queue_lnk *lnk;
};

static void queue_add(queue_lnk *new, queue_lnk *prev, queue_lnk *next);
static void queue_del(queue_lnk *prev, queue_lnk *next);

void
queue_init(queue_t **queue)
{
    *queue = emalloc(sizeof(queue_t));
    pthread_mutex_init(&(*queue)->mtx, NULL);
    pthread_cond_init(&(*queue)->cond, NULL);
    (*queue)->lnk = emalloc(sizeof(queue_lnk));
    (*queue)->lnk->prev = (*queue)->lnk->next = (*queue)->lnk;
}

void
queue_destroy(queue_t **queue)
{
    pthread_mutex_destroy(&(*queue)->mtx);
    pthread_cond_destroy(&(*queue)->cond);
    free((*queue)->lnk);
    free(*queue);
    *queue = NULL;
}

void
queue_add(queue_lnk *new, queue_lnk *prev, queue_lnk *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void
enqueue(queue_lnk *new, queue_t *queue)
{
    pthread_mutex_lock(&queue->mtx);
    queue_add(new, queue->lnk->prev, queue->lnk);
    pthread_mutex_unlock(&queue->mtx);
    pthread_cond_signal(&queue->cond);
}

void
queue_del(queue_lnk *prev, queue_lnk *next)
{
    next->prev = prev;
    prev->next = next;
}

queue_lnk *
dequeue_lnk(queue_t *queue)
{
    queue_lnk *first;

    pthread_mutex_lock(&queue->mtx);

    while (queue_empty(queue))
        pthread_cond_wait(&queue->cond, &queue->mtx);

    first = queue->lnk->next;
    queue_del(queue->lnk, queue->lnk->next->next);
    first->prev = first->next = NULL;
    pthread_mutex_unlock(&queue->mtx);
    return first;
}

int
queue_empty(const queue_t *queue)
{
    return queue->lnk->next == queue->lnk;
}
