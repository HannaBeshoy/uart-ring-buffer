#include <math.h>
#include <stdlib.h>
#include "demos.h"

int hash_golden(int key, int n)
{
    /* p. 107: h(i) = floor(n * (i * 1.618 mod 1)) */
    double x = key * 1.618;
    return (int)floor(n * (x - floor(x)));
}

void ot_init(open_table_t *t)
{
    for (int i = 0; i < HT_SIZE; i++) { t->state[i] = SLOT_EMPTY; t->value[i] = 0; }
}

int ot_insert(open_table_t *t, int key, int start)
{
    /* linear probing: start, start+1, ... ; a deleted slot can be reused */
    for (int j = 0; j < HT_SIZE; j++) {
        int i = (start + j) % HT_SIZE;
        if (t->state[i] != SLOT_USED) { t->state[i] = SLOT_USED; t->value[i] = key; return i; }
    }
    return -1;                                    /* table full */
}

int ot_find(const open_table_t *t, int key, int start)
{
    /* stop at an EMPTY slot, but walk over DELETED ones: that is why a
       deletion must leave a "deleted" mark instead of "empty" (p. 106) */
    for (int j = 0; j < HT_SIZE; j++) {
        int i = (start + j) % HT_SIZE;
        if (t->state[i] == SLOT_EMPTY) return -1;
        if (t->state[i] == SLOT_USED && t->value[i] == key) return i;
    }
    return -1;
}

bool ot_delete(open_table_t *t, int key, int start)
{
    int i = ot_find(t, key, start);
    if (i < 0) return false;
    t->state[i] = SLOT_DELETED;
    return true;
}

void ct_init(chained_table_t *t) { for (int i = 0; i < HT_SIZE; i++) t->bucket[i] = NULL; }

void ct_insert(chained_table_t *t, int key)
{
    int b = hash_golden(key, HT_SIZE);
    chain_t *c = malloc(sizeof *c);
    if (!c) return;
    c->key = key; c->next = t->bucket[b]; t->bucket[b] = c;
}

bool ct_contains(const chained_table_t *t, int key)
{
    for (chain_t *c = t->bucket[hash_golden(key, HT_SIZE)]; c; c = c->next) if (c->key == key) return true;
    return false;
}

void ct_free(chained_table_t *t)
{
    for (int i = 0; i < HT_SIZE; i++)
        while (t->bucket[i]) { chain_t *n = t->bucket[i]->next; free(t->bucket[i]); t->bucket[i] = n; }
}
