#include <stdlib.h>
#include "demos.h"

static node_t *new_node(int value, node_t *next)
{
    node_t *n = malloc(sizeof *n);
    if (n) { n->value = value; n->next = next; }
    return n;
}

node_t *list_push_front(node_t *head, int value)
{
    node_t *n = new_node(value, head);
    return n ? n : head;
}

node_t *list_insert_sorted(node_t *head, int value)
{
    if (head == NULL || value <= head->value) return list_push_front(head, value);
    node_t *p = head;
    while (p->next && p->next->value < value) p = p->next;
    node_t *n = new_node(value, p->next);
    if (n) p->next = n;
    return head;
}

node_t *list_remove(node_t *head, int value, bool *removed)
{
    node_t **link = &head;                /* pointer to the pointer that may change */
    *removed = false;
    while (*link && (*link)->value != value) link = &(*link)->next;
    if (*link) {
        node_t *dead = *link;
        *link = dead->next;
        free(dead);
        *removed = true;
    }
    return head;
}

node_t *list_reverse(node_t *head)
{
    node_t *prev = NULL;
    while (head) { node_t *next = head->next; head->next = prev; prev = head; head = next; }
    return prev;
}

size_t list_length(const node_t *head)
{
    size_t n = 0;
    for (; head; head = head->next) n++;
    return n;
}

size_t list_to_array(const node_t *head, int *out, size_t max)
{
    size_t n = 0;
    for (; head && n < max; head = head->next) out[n++] = head->value;
    return n;
}

void list_free(node_t *head)
{
    while (head) { node_t *next = head->next; free(head); head = next; }
}
