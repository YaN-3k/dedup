#ifndef QUEUE_H__
#define QUEUE_H__

#include <stddef.h>

struct queue_head {
    struct queue_head *next, *prev;
};

#define QUEUE_HEAD_INIT(name) { &(name), &(name) }
#define queue_head_init(head) (head)->next = (head)->prev = head;
#define queue_entry(ptr, type, member) \
    (type *)((char *)(ptr) - offsetof(type, member))
#define dequeue_entry(head, type, member) \
    queue_entry(dequeue(head), type, member)

void enqueue(struct queue_head *new, struct queue_head *head);
struct queue_head * dequeue(struct queue_head *head);
int queue_empty(const struct queue_head *head);

#endif
