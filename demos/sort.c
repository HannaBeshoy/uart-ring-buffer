#include <stdlib.h>
#include <string.h>
#include "demos.h"

#define LESS(x, y) (st->comparisons++, (x) < (y))
static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

void bubble_sort(int *a, size_t n, sort_stats_t *st)
{
    /* tuned (p. 92): stop when a pass makes no swap; the last swap position
       marks where the sorted tail begins */
    size_t end = n;
    while (end > 1) {
        size_t last = 0;
        for (size_t j = 0; j + 1 < end; j++)
            if (LESS(a[j + 1], a[j])) { swap(&a[j], &a[j + 1]); st->moves += 3; last = j + 1; }
        end = last;
    }
}

void insertion_sort(int *a, size_t n, sort_stats_t *st)
{
    for (size_t i = 1; i < n; i++) {
        int x = a[i];
        size_t j = i;
        while (j > 0 && LESS(x, a[j - 1])) { a[j] = a[j - 1]; st->moves++; j--; }
        a[j] = x; st->moves++;
    }
}

void ranking_sort(int *a, size_t n, sort_stats_t *st)
{
    /* p. 94: R[i] = number of elements that must come before a[i] */
    size_t *r = calloc(n, sizeof *r);
    int *b = malloc(n * sizeof *b);
    for (size_t i = 0; i + 1 < n; i++)
        for (size_t j = i + 1; j < n; j++) {
            st->comparisons++;
            if (a[i] > a[j]) r[i]++; else r[j]++;
        }
    for (size_t i = 0; i < n; i++) { b[r[i]] = a[i]; st->moves++; }
    memcpy(a, b, n * sizeof *a);
    free(r); free(b);
}

static void merge(int *a, int *tmp, size_t lo, size_t mid, size_t hi, sort_stats_t *st)
{
    size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = LESS(a[j], a[i]) ? a[j++] : a[i++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi) tmp[k++] = a[j++];
    for (k = lo; k < hi; k++) { a[k] = tmp[k]; st->moves++; }
}

static void msort(int *a, int *tmp, size_t lo, size_t hi, sort_stats_t *st)
{
    if (hi - lo < 2) return;
    size_t mid = lo + (hi - lo) / 2;
    msort(a, tmp, lo, mid, st);
    msort(a, tmp, mid, hi, st);
    merge(a, tmp, lo, mid, hi, st);
}

void merge_sort(int *a, size_t n, sort_stats_t *st)
{
    int *tmp = malloc(n * sizeof *tmp);
    msort(a, tmp, 0, n, st);
    free(tmp);
}

void merge_sort_iterative(int *a, size_t n, sort_stats_t *st)
{
    /* p. 96: merge runs of width 1, 2, 4, ... bottom-up, no recursion */
    int *tmp = malloc(n * sizeof *tmp);
    for (size_t w = 1; w < n; w *= 2)
        for (size_t lo = 0; lo + w < n; lo += 2 * w) {
            size_t hi = lo + 2 * w < n ? lo + 2 * w : n;
            merge(a, tmp, lo, lo + w, hi, st);
        }
    free(tmp);
}

static void qsort_rec(int *a, long lo, long hi, sort_stats_t *st)
{
    while (lo < hi) {
        int pivot = a[lo + (hi - lo) / 2];          /* middle element as pivot */
        long i = lo, j = hi;
        while (i <= j) {                            /* Hoare-style partition */
            while (LESS(a[i], pivot)) i++;
            while (LESS(pivot, a[j])) j--;
            if (i <= j) { swap(&a[i], &a[j]); st->moves += 3; i++; j--; }
        }
        if (j - lo < hi - i) { qsort_rec(a, lo, j, st); lo = i; }   /* recurse on the smaller side */
        else { qsort_rec(a, i, hi, st); hi = j; }
    }
}

void quick_sort(int *a, size_t n, sort_stats_t *st) { if (n > 1) qsort_rec(a, 0, (long)n - 1, st); }

static void sift_down(int *a, size_t i, size_t n, sort_stats_t *st)
{
    for (;;) {
        size_t c = 2 * i + 1;
        if (c >= n) return;
        if (c + 1 < n && LESS(a[c], a[c + 1])) c++;     /* the larger child (max-heap) */
        if (!LESS(a[i], a[c])) return;
        swap(&a[i], &a[c]); st->moves += 3;
        i = c;
    }
}

void heap_sort(int *a, size_t n, sort_stats_t *st)
{
    for (size_t i = n / 2; i-- > 0;) sift_down(a, i, n, st);
    for (size_t end = n; end > 1; end--) {
        swap(&a[0], &a[end - 1]); st->moves += 3;
        sift_down(a, 0, end - 1, st);
    }
}

void bin_sort(int *a, size_t n, int m)
{
    /* p. 100: one queue per key value; counting sizes first gives the same
       result without linked lists: O(n + m) */
    size_t *start = calloc((size_t)m + 1, sizeof *start);
    int *b = malloc(n * sizeof *b);
    for (size_t i = 0; i < n; i++) start[a[i] + 1]++;
    for (int k = 0; k < m; k++) start[k + 1] += start[k];
    for (size_t i = 0; i < n; i++) b[start[a[i]]++] = a[i];   /* stable */
    memcpy(a, b, n * sizeof *a);
    free(start); free(b);
}

void odd_even_sort(int *a, size_t n, int *rounds)
{
    /* p. 214: in round k every even (or odd) pair is compared at the same
       time, so n/2 processors finish in n rounds: O(n) parallel time */
    *rounds = 0;
    for (size_t k = 0; k < n; k++) {
        for (size_t i = k % 2; i + 1 < n; i += 2)         /* the "pfor": independent pairs */
            if (a[i] > a[i + 1]) swap(&a[i], &a[i + 1]);
        (*rounds)++;
    }
}

long binary_search(const int *a, size_t n, int key, int *steps)
{
    size_t lo = 0, hi = n;
    *steps = 0;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        (*steps)++;
        if (a[mid] == key) return (long)mid;
        if (a[mid] < key) lo = mid + 1; else hi = mid;
    }
    return -1;
}
