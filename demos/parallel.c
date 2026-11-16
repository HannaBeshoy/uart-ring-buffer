/*
 * parallel.c - parallel algorithms on the PRAM model (pp. 209-217).
 *
 * par_min and par_sum follow the slides' pseudocode: in each round the
 * "pfor" body runs for every pair at the same time, and the distance between
 * partners doubles. Here the pfor is an ordinary loop, and the statistics
 * record what a parallel machine would need: the number of rounds (parallel
 * time, O(log n)), the largest number of pairs in one round (processors,
 * n/2) and the total number of operations (work, n - 1).
 *
 * par_sum_threads runs the same idea on real threads: each thread adds its
 * own slice, with no shared variable to protect, and the main thread adds
 * the partial sums.
 */
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "demos.h"

int par_min(const int *a, size_t n, par_stats_t *st)
{
    memset(st, 0, sizeof *st);
    if (n == 0) return 0;
    int *m = malloc(n * sizeof *m);
    if (!m) return 0;
    memcpy(m, a, n * sizeof *m);
    for (size_t k = 1; k < n; k *= 2) {
        int pairs = 0;
        for (size_t i = 0; i + k < n; i += 2 * k) {       /* pfor: the pairs do not overlap */
            if (m[i] > m[i + k]) m[i] = m[i + k];
            pairs++;
        }
        st->rounds++;
        st->operations += pairs;
        if (pairs > st->processors) st->processors = pairs;
    }
    int result = m[0];
    free(m);
    return result;
}

long par_sum(const int *a, size_t n, par_stats_t *st)
{
    memset(st, 0, sizeof *st);
    if (n == 0) return 0;
    long *m = malloc(n * sizeof *m);
    if (!m) return 0;
    for (size_t i = 0; i < n; i++) m[i] = a[i];
    for (size_t k = 1; k < n; k *= 2) {
        int pairs = 0;
        for (size_t i = 0; i + k < n; i += 2 * k) {       /* pfor */
            m[i] += m[i + k];
            pairs++;
        }
        st->rounds++;
        st->operations += pairs;
        if (pairs > st->processors) st->processors = pairs;
    }
    long result = m[0];
    free(m);
    return result;
}

typedef struct { const int *a; size_t from, to; long sum; } slice_t;

static void *add_slice(void *arg)
{
    slice_t *s = arg;
    long sum = 0;
    for (size_t i = s->from; i < s->to; i++) sum += s->a[i];
    s->sum = sum;                                         /* each thread writes only its own slot */
    return NULL;
}

long par_sum_threads(const int *a, size_t n, int threads)
{
    if (threads < 1) threads = 1;
    if (threads > 16) threads = 16;
    pthread_t id[16];
    slice_t slice[16];
    bool started[16];
    long total = 0;
    for (int t = 0; t < threads; t++) {
        slice[t] = (slice_t){ a, n * (size_t)t / (size_t)threads, n * (size_t)(t + 1) / (size_t)threads, 0 };
        started[t] = pthread_create(&id[t], NULL, add_slice, &slice[t]) == 0;
        if (!started[t]) add_slice(&slice[t]);            /* no thread available: do the slice here */
    }
    for (int t = 0; t < threads; t++) {
        if (started[t]) pthread_join(id[t], NULL);
        total += slice[t].sum;
    }
    return total;
}
