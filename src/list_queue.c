#include "list_queue.h"
#include <stdlib.h>

void lq_init(list_queue_t *q) { q->first = q->last = NULL; q->count = 0; }

bool lq_put(list_queue_t *q, uint8_t value)
{
    lq_node_t *n = malloc(sizeof *n);
    if (n == NULL) return false;
    n->value = value;
    n->next = NULL;
    if (q->last) q->last->next = n; else q->first = n;
    q->last = n;
    q->count++;
    return true;
}

bool lq_get(list_queue_t *q, uint8_t *value)
{
    lq_node_t *n = q->first;
    if (n == NULL) return false;
    *value = n->value;
    q->first = n->next;
    if (q->first == NULL) q->last = NULL;
    free(n);
    q->count--;
    return true;
}

size_t lq_count(const list_queue_t *q) { return q->count; }

void lq_free(list_queue_t *q)
{
    uint8_t v;
    while (lq_get(q, &v)) { }
}
