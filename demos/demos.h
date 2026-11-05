/*
 * demos.h - one small, tested C implementation per lecture topic of
 * Data Structures and Algorithms (University of Debrecen, 2026 slides).
 * Page numbers refer to DataStructures-2026.pdf.
 */
#ifndef DEMOS_H
#define DEMOS_H
#include <stdbool.h>
#include <stddef.h>

/* ---- algorithms.c: analysis of algorithms (pp. 18-41) ---------------------------- */
typedef struct { int min, max; long comparisons; } minmax_t;
minmax_t min_max_pairs(const int *a, size_t n);           /* about 3n/2 comparisons (p. 22) */
double horner(const double *coef, size_t degree, double x, long *mults); /* p. 39 */
unsigned long long fast_pow(unsigned long long base, unsigned e, long *mults); /* p. 40 */

/* ---- linked_list.c: list data structures (pp. 55-62) ------------------------------ */
typedef struct node { int value; struct node *next; } node_t;
node_t *list_push_front(node_t *head, int value);
node_t *list_insert_sorted(node_t *head, int value);
node_t *list_remove(node_t *head, int value, bool *removed);
node_t *list_reverse(node_t *head);
size_t list_length(const node_t *head);
size_t list_to_array(const node_t *head, int *out, size_t max);
void list_free(node_t *head);

/* ---- self_organising.c: self-organising list (p. 87) ------------------------------ */
typedef enum { SO_MOVE_TO_FRONT, SO_TRANSPOSE, SO_COUNT } so_rule_t;
typedef struct { int key[32]; int count[32]; size_t n; so_rule_t rule; } so_list_t;
void so_init(so_list_t *l, so_rule_t rule, const int *keys, size_t n);
int so_find(so_list_t *l, int key);                      /* returns comparisons used, -1 if absent */

/* ---- stack.c: stacks (pp. 80-84) ------------------------------------------------------ */
typedef struct { double data[64]; size_t top; } dstack_t;
void stack_init(dstack_t *s);
bool stack_push(dstack_t *s, double v);
bool stack_pop(dstack_t *s, double *v);
bool brackets_balanced(const char *text);
bool rpn_eval(const char *expr, double *result);         /* "3 4 + 2 *" */

/* ---- array_queue.c: the lecture's cyclic array queue (pp. 71-79) ---------------- */
typedef struct { int data[8]; size_t first, count; } aqueue_t; /* count kept, so all 8 slots used */
void aq_init(aqueue_t *q);
bool aq_enqueue(aqueue_t *q, int v);
bool aq_dequeue(aqueue_t *q, int *v);

/* ---- sparse.c: sparse matrices in triplet form (pp. 66-68) ------------------------ */
typedef struct { int row, col; double value; } triplet_t;
typedef struct { int rows, cols; size_t n; triplet_t t[64]; } sparse_t;
bool sparse_set(sparse_t *m, int row, int col, double value);
double sparse_get(const sparse_t *m, int row, int col);
void sparse_mul_vec(const sparse_t *m, const double *x, double *y);

/* ---- sort.c: sorting algorithms (pp. 89-100) ----------------------------------------- */
typedef struct { long comparisons, moves; } sort_stats_t;
void bubble_sort(int *a, size_t n, sort_stats_t *st);    /* tuned: stops when no swap */
void insertion_sort(int *a, size_t n, sort_stats_t *st);
void ranking_sort(int *a, size_t n, sort_stats_t *st);
void merge_sort(int *a, size_t n, sort_stats_t *st);     /* recursive */
void merge_sort_iterative(int *a, size_t n, sort_stats_t *st);
void quick_sort(int *a, size_t n, sort_stats_t *st);
void heap_sort(int *a, size_t n, sort_stats_t *st);
void bin_sort(int *a, size_t n, int m);                  /* keys 0..m-1, stable */
void odd_even_sort(int *a, size_t n, int *rounds);       /* parallel bubble sort, simulated (p. 214) */
long binary_search(const int *a, size_t n, int key, int *steps);

/* ---- heap.c: heap and priority queue (pp. 122-137) ---------------------------------- */
typedef struct { int data[256]; size_t n; } heap_t;      /* minimum heap, index 0 = root */
void heap_init(heap_t *h);
bool heap_insert(heap_t *h, int v);
bool heap_pop_min(heap_t *h, int *v);
bool heap_is_valid(const heap_t *h);

/* ---- hash.c: hash tables (pp. 101-109) ------------------------------------------------ */
#define HT_SIZE 10
typedef enum { SLOT_EMPTY = 0, SLOT_USED = 1, SLOT_DELETED = -1 } slot_state_t;
typedef struct { slot_state_t state[HT_SIZE]; int value[HT_SIZE]; } open_table_t;
int hash_golden(int key, int n);                         /* floor(n * frac(key * 1.618)) */
void ot_init(open_table_t *t);
int ot_insert(open_table_t *t, int key, int start);      /* linear probing from start; slot or -1 */
int ot_find(const open_table_t *t, int key, int start);
bool ot_delete(open_table_t *t, int key, int start);

typedef struct chain { int key; struct chain *next; } chain_t;
typedef struct { chain_t *bucket[HT_SIZE]; } chained_table_t;  /* direct addressing with lists */
void ct_init(chained_table_t *t);
void ct_insert(chained_table_t *t, int key);
bool ct_contains(const chained_table_t *t, int key);
void ct_free(chained_table_t *t);

/* ---- bst.c: binary search trees and traversals (pp. 138-161) ---------------------- */
typedef struct tnode { int key; struct tnode *left, *right, *parent; } tnode_t;
tnode_t *bst_insert(tnode_t *root, int key);
tnode_t *bst_find(tnode_t *root, int key);
tnode_t *bst_min(tnode_t *t);
tnode_t *bst_next(tnode_t *t);                           /* in-order successor (p. 145) */
tnode_t *bst_delete(tnode_t *root, int key);
size_t bst_inorder(const tnode_t *t, int *out, size_t i);
size_t bst_preorder(const tnode_t *t, int *out, size_t i);
size_t bst_postorder(const tnode_t *t, int *out, size_t i);
size_t bst_breadth_first(tnode_t *root, int *out);
int bst_height(const tnode_t *t);
void bst_free(tnode_t *t);

/* ---- avl.c: self-balancing AVL tree (pp. 162-178) --------------------------------- */
typedef struct anode { int key, height; struct anode *left, *right; } anode_t;
anode_t *avl_insert(anode_t *root, int key, int *rotations);
bool avl_is_balanced(const anode_t *t);
int avl_height(const anode_t *t);
size_t avl_inorder(const anode_t *t, int *out, size_t i);
void avl_free(anode_t *t);

/* ---- graph.c: graphs (pp. 193-208) --------------------------------------------------- */
#define G_MAX 16
typedef struct { int n; int w[G_MAX][G_MAX]; } graph_t; /* adjacency matrix; 0 = no edge */
void graph_init(graph_t *g, int n);
void graph_add_edge(graph_t *g, int a, int b, int weight); /* undirected */
int graph_bfs(const graph_t *g, int start, int *order);
int graph_dfs(const graph_t *g, int start, int *order);
bool graph_has_euler_circuit(const graph_t *g);          /* connected, every degree even (p. 207) */
bool graph_is_bipartite(const graph_t *g);               /* 2-colourable (p. 207) */
void graph_dijkstra(const graph_t *g, int source, int *dist); /* shortest paths (p. 208) */

#endif
