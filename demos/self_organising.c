#include "demos.h"

void so_init(so_list_t *l, so_rule_t rule, const int *keys, size_t n)
{
    l->n = n; l->rule = rule;
    for (size_t i = 0; i < n; i++) { l->key[i] = keys[i]; l->count[i] = 0; }
}

static void swap_at(so_list_t *l, size_t i, size_t j)
{
    int k = l->key[i]; l->key[i] = l->key[j]; l->key[j] = k;
    int c = l->count[i]; l->count[i] = l->count[j]; l->count[j] = c;
}

int so_find(so_list_t *l, int key)
{
    size_t i = 0;
    while (i < l->n && l->key[i] != key) i++;
    if (i == l->n) return -1;
    int comparisons = (int)i + 1;
    switch (l->rule) {
    case SO_MOVE_TO_FRONT:                       /* rule 1: most recent first */
        for (; i > 0; i--) swap_at(l, i, i - 1);
        break;
    case SO_TRANSPOSE:                           /* rule 3: move up one */
        if (i > 0) swap_at(l, i, i - 1);
        break;
    case SO_COUNT:                               /* rule 2: order by frequency */
        l->count[i]++;
        while (i > 0 && l->count[i - 1] < l->count[i]) { swap_at(l, i, i - 1); i--; }
        break;
    }
    return comparisons;
}
