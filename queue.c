#include "queue.h"

static void
queue_add(struct queue_head *new,
          struct queue_head *prev,
          struct queue_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void
enqueue(struct queue_head *new, struct queue_head *head)
{
    queue_add(new, head->prev, head);
}

static void
queue_del(struct queue_head *prev, struct queue_head *next)
{
    next->prev = prev;
    prev->next = next;
}

struct queue_head *
dequeue(struct queue_head *head)
{
    struct queue_head *first = head->next;
    queue_del(head, head->next->next);
    first->prev = first->next = NULL;
    return first;
}

int
queue_empty(const struct queue_head *head)
{
    return head->next == head;
}
