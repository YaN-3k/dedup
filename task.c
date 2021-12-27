#include "task.h"

#include <stdlib.h>

#include "util.h"

void
task_head_init(struct task_head *tasks)
{
    pthread_mutex_init(&tasks->mtx, NULL);
    pthread_cond_init(&tasks->cond, NULL);
    queue_head_init(&tasks->head);
}

void
task_add(const char *fpath, struct task_head *tasks)
{
    struct task_entry *entry;

    entry = emalloc(sizeof(struct task_entry));
    entry->fpath = fpath;

    pthread_mutex_lock(&tasks->mtx);
    enqueue(&entry->head, &tasks->head);
    pthread_mutex_unlock(&tasks->mtx);
    pthread_cond_signal(&tasks->cond);
}

struct task_entry *
task_get(struct task_head *tasks)
{
    struct task_entry *entry;

    pthread_mutex_lock(&tasks->mtx);

    while (queue_empty(&tasks->head))
        pthread_cond_wait(&tasks->cond, &tasks->mtx);

    entry = dequeue_entry(&tasks->head, struct task_entry, head);
    pthread_mutex_unlock(&tasks->mtx);

    return entry;
}

void
task_free(struct task_entry *entry)
{
    free((char *)entry->fpath);
    free(entry);
}

void
task_head_free(struct task_head *tasks)
{
    pthread_cond_destroy(&tasks->cond);
    pthread_mutex_destroy(&tasks->mtx);
}
