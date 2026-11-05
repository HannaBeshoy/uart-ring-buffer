#include "demos.h"

void heap_init(heap_t *h) { h->n = 0; }

bool heap_insert(heap_t *h, int v)
{
    /* p. 124: put it at the end, then swap upward while smaller than the parent */
    if (h->n == sizeof h->data / sizeof h->data[0]) return false;
    size_t i = h->n++;
    while (i > 0 && h->data[(i - 1) / 2] > v) { h->data[i] = h->data[(i - 1) / 2]; i = (i - 1) / 2; }
    h->data[i] = v;
    return true;
}

bool heap_pop_min(heap_t *h, int *v)
{
    /* p. 128: the root is the minimum; move the last item to the root and sift down */
    if (h->n == 0) return false;
    *v = h->data[0];
    int x = h->data[--h->n];
    size_t i = 0;
    for (;;) {
        size_t c = 2 * i + 1;
        if (c >= h->n) break;
        if (c + 1 < h->n && h->data[c + 1] < h->data[c]) c++;
        if (x <= h->data[c]) break;
        h->data[i] = h->data[c];
        i = c;
    }
    if (h->n) h->data[i] = x;
    return true;
}

bool heap_is_valid(const heap_t *h)
{
    for (size_t i = 1; i < h->n; i++) if (h->data[(i - 1) / 2] > h->data[i]) return false;
    return true;
}
