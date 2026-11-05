/*
 * list_queue.h - the same FIFO as a singly linked list (lecture: queues,
 * linked representation). Kept for comparison: it grows without limit, but
 * every byte costs a node (data + next pointer) and a malloc, which is why it
 * is a poor fit for an interrupt on a 2 KB microcontroller.
 */
#ifndef LIST_QUEUE_H
#define LIST_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct lq_node {
    uint8_t value;
    struct lq_node *next;
} lq_node_t;

typedef struct {
    lq_node_t *first;   /* dequeue here */
    lq_node_t *last;    /* enqueue here */
    size_t count;
} list_queue_t;

void lq_init(list_queue_t *q);
bool lq_put(list_queue_t *q, uint8_t value);   /* false if malloc fails */
bool lq_get(list_queue_t *q, uint8_t *value);
size_t lq_count(const list_queue_t *q);
void lq_free(list_queue_t *q);

#endif
