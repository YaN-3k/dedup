typedef struct queue_lnk {
    struct queue_lnk *next, *prev;
} queue_lnk;

typedef struct queue_t queue_t;

#define queue_entry(ptr, type, member) \
    (type *)((char *)(ptr) - offsetof(type, member))
#define dequeue(queue, type, member) \
    queue_entry(dequeue_lnk(queue), type, member)

void queue_init(queue_t **queue);
void enqueue(queue_lnk *new, queue_t *queue);
queue_lnk *dequeue_lnk(queue_t *queue);
int queue_empty(const queue_t *queue);
void queue_destroy(queue_t **queue);
