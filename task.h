#ifndef TASK_H__
#define TASK_H__

#include "queue.h"

#include <pthread.h>

struct task_head {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    struct queue_head head;
};

struct task_entry {
    const char *fpath;
    struct queue_head head;
};

void task_head_init(struct task_head *tasks);
void *process_file(void *fpath);
void task_add(const char *fpath, struct task_head *tasks);
struct task_entry *task_get(struct task_head *tasks);
void task_free(struct task_entry *entry);
void task_head_free(struct task_head *tasks);

#endif
