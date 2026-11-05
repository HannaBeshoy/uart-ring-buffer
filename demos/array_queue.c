#include "demos.h"

#define AQ_CAP (sizeof ((aqueue_t *)0)->data / sizeof(int))

void aq_init(aqueue_t *q) { q->first = 0; q->count = 0; }

bool aq_enqueue(aqueue_t *q, int v)
{
    if (q->count == AQ_CAP) return false;            /* the lecture's overflow check */
    q->data[(q->first + q->count) % AQ_CAP] = v;
    q->count++;
    return true;
}

bool aq_dequeue(aqueue_t *q, int *v)
{
    if (q->count == 0) return false;
    *v = q->data[q->first];
    q->first = (q->first + 1) % AQ_CAP;
    q->count--;
    return true;
}
