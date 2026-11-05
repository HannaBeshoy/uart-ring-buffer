/* Tests for every lecture demo in demos/: run with `make test`. */
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "demos.h"

static int sorted(const int *a, size_t n) { for (size_t i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0; return 1; }
static int cmp_int(const void *x, const void *y) { int a = *(const int *)x, b = *(const int *)y; return (a > b) - (a < b); }

TEST(analysis_min_max_horner_power)
{
    int a[] = { 7, 3, 1, 5, 3, 6, 9, 2, 4 };                 /* the lecture's example data */
    minmax_t r = min_max_pairs(a, 9);
    CHECK(r.min == 1 && r.max == 9);
    CHECK(r.comparisons == 12);                              /* 3*(9-1)/2 instead of 2*(9-1) = 16 */
    int b[] = { 4, 8 };
    r = min_max_pairs(b, 2);
    CHECK(r.min == 4 && r.max == 8 && r.comparisons == 1);
    double c[] = { 2, -6, 2, -1 };                           /* 2x^3 - 6x^2 + 2x - 1 */
    long m;
    CHECK(horner(c, 3, 3, &m) == 5 && m == 3);               /* 54 - 54 + 6 - 1 */
    CHECK(fast_pow(3, 13, &m) == 1594323ULL && m == 6);      /* 6 instead of 12 multiplications */
    CHECK(fast_pow(2, 0, &m) == 1 && m == 0);
}

TEST(linked_list_operations)
{
    node_t *l = NULL;
    int out[16];
    bool removed;
    int in[] = { 5, 1, 9, 3, 7 };
    for (int i = 0; i < 5; i++) l = list_insert_sorted(l, in[i]);
    CHECK(list_to_array(l, out, 16) == 5 && memcmp(out, (int[]){ 1, 3, 5, 7, 9 }, sizeof(int) * 5) == 0);
    l = list_remove(l, 1, &removed);  CHECK(removed);      /* the head */
    l = list_remove(l, 9, &removed);  CHECK(removed);      /* the tail */
    l = list_remove(l, 4, &removed);  CHECK(!removed);
    l = list_reverse(l);
    CHECK(list_to_array(l, out, 16) == 3 && memcmp(out, (int[]){ 7, 5, 3 }, sizeof(int) * 3) == 0);
    l = list_push_front(l, 0);
    CHECK(list_length(l) == 4 && l->value == 0);
    list_free(l);
}

TEST(self_organising_list)
{
    int keys[] = { 1, 2, 3, 4, 5 };
    so_list_t l;
    so_init(&l, SO_MOVE_TO_FRONT, keys, 5);
    CHECK(so_find(&l, 5) == 5 && l.key[0] == 5);
    CHECK(so_find(&l, 5) == 1);                              /* now found at once */
    so_init(&l, SO_TRANSPOSE, keys, 5);
    so_find(&l, 4);
    CHECK(l.key[2] == 4 && l.key[3] == 3);
    so_init(&l, SO_COUNT, keys, 5);
    so_find(&l, 3); so_find(&l, 3); so_find(&l, 2);
    CHECK(l.key[0] == 3 && l.key[1] == 2);
    CHECK(so_find(&l, 42) == -1);
}

TEST(stack_applications)
{
    double r;
    CHECK(brackets_balanced("a[i] = (b + {c * d})"));
    CHECK(!brackets_balanced("(]"));
    CHECK(!brackets_balanced("(("));
    CHECK(!brackets_balanced(")("));
    CHECK(rpn_eval("3 4 + 2 *", &r) && r == 14);
    CHECK(rpn_eval("5 1 2 + 4 * + 3 -", &r) && r == 14);
    CHECK(!rpn_eval("1 +", &r));
    CHECK(!rpn_eval("1 0 /", &r));
    dstack_t s; stack_init(&s);
    for (int i = 0; i < 64; i++) CHECK(stack_push(&s, i));
    CHECK(!stack_push(&s, 64));                               /* overflow */
}

TEST(cyclic_array_queue)
{
    aqueue_t q; int v, ok = 1;
    aq_init(&q);
    CHECK(!aq_dequeue(&q, &v));
    for (int i = 0; i < 8; i++) CHECK(aq_enqueue(&q, i));
    CHECK(!aq_enqueue(&q, 8));                                /* overflow check */
    for (int i = 0; i < 100; i++) { ok &= aq_dequeue(&q, &v) && v == i; ok &= aq_enqueue(&q, i + 8); }
    CHECK(ok);                                                /* O(1) enqueue and dequeue across the wrap */
}

TEST(sparse_matrix)
{
    sparse_t m = { .rows = 4, .cols = 4, .n = 0 };
    sparse_set(&m, 0, 0, 2); sparse_set(&m, 3, 1, 5); sparse_set(&m, 1, 2, -1); sparse_set(&m, 0, 3, 4);
    CHECK(m.n == 4 && m.t[0].row == 0 && m.t[1].col == 3);   /* kept sorted by row, column */
    CHECK(sparse_get(&m, 3, 1) == 5 && sparse_get(&m, 2, 2) == 0);
    double x[] = { 1, 2, 3, 4 }, y[4];
    sparse_mul_vec(&m, x, y);
    CHECK(y[0] == 18 && y[1] == -3 && y[2] == 0 && y[3] == 10);
    sparse_set(&m, 3, 1, 0);                                  /* setting zero removes it */
    CHECK(m.n == 3 && sparse_get(&m, 3, 1) == 0);
}

typedef void (*sorter_t)(int *, size_t, sort_stats_t *);

TEST(sorts_agree_with_qsort)
{
    sorter_t sorters[] = { bubble_sort, insertion_sort, ranking_sort, merge_sort, merge_sort_iterative, quick_sort, heap_sort };
    size_t sizes[] = { 0, 1, 2, 9, 100, 1000 };
    int ok = 1;
    srand(7);
    for (size_t s = 0; s < sizeof sizes / sizeof *sizes; s++) {
        size_t n = sizes[s];
        int *src = malloc((n + 1) * sizeof(int)), *want = malloc((n + 1) * sizeof(int)), *a = malloc((n + 1) * sizeof(int));
        for (size_t i = 0; i < n; i++) src[i] = rand() % 50;  /* many duplicates */
        memcpy(want, src, n * sizeof(int));
        qsort(want, n, sizeof(int), cmp_int);
        for (size_t k = 0; k < sizeof sorters / sizeof *sorters; k++) {
            sort_stats_t st = { 0, 0 };
            memcpy(a, src, n * sizeof(int));
            sorters[k](a, n, &st);
            ok &= memcmp(a, want, n * sizeof(int)) == 0;
        }
        memcpy(a, src, n * sizeof(int)); bin_sort(a, n, 50); ok &= memcmp(a, want, n * sizeof(int)) == 0;
        int rounds; memcpy(a, src, n * sizeof(int)); odd_even_sort(a, n, &rounds); ok &= sorted(a, n) && rounds == (int)n;
        free(src); free(want); free(a);
    }
    CHECK(ok);
}

TEST(sort_costs_match_the_theory)
{
    int a[1000], b[1000];
    sort_stats_t bub = { 0, 0 }, ins = { 0, 0 }, mer = { 0, 0 };
    for (int i = 0; i < 1000; i++) a[i] = i;                  /* already sorted */
    memcpy(b, a, sizeof a); bubble_sort(b, 1000, &bub);
    memcpy(b, a, sizeof a); insertion_sort(b, 1000, &ins);
    CHECK(bub.comparisons == 999 && bub.moves == 0);          /* tuned bubble: one pass */
    CHECK(ins.comparisons == 999);
    for (int i = 0; i < 1000; i++) a[i] = 1000 - i;           /* reversed: the worst case */
    memcpy(b, a, sizeof a); insertion_sort(b, 1000, &ins);
    memcpy(b, a, sizeof a); merge_sort(b, 1000, &mer);
    CHECK(ins.comparisons - 999 == 999L * 1000 / 2);          /* n(n-1)/2 */
    CHECK(mer.comparisons <= 1000L * 10);                     /* n log2 n, log2 1000 < 10 */
}

TEST(binary_search_steps)
{
    int a[1024], steps;
    for (int i = 0; i < 1024; i++) a[i] = 2 * i;
    CHECK(binary_search(a, 1024, 1000, &steps) == 500 && steps <= 11);
    CHECK(binary_search(a, 1024, 1001, &steps) == -1 && steps <= 11);   /* log2 1024 + 1 */
    CHECK(binary_search(a, 0, 5, &steps) == -1 && steps == 0);
}

TEST(heap_priority_queue)
{
    heap_t h; int v, ok = 1, last = INT_MIN;
    heap_init(&h);
    int in[] = { 7, 3, 1, 5, 3, 6, 9, 2, 4 };
    for (int i = 0; i < 9; i++) { heap_insert(&h, in[i]); ok &= heap_is_valid(&h); }
    CHECK(ok && h.data[0] == 1);
    for (int i = 0; i < 9; i++) { ok &= heap_pop_min(&h, &v) && v >= last && heap_is_valid(&h); last = v; }
    CHECK(ok && !heap_pop_min(&h, &v));
}

TEST(hash_table_lecture_example)
{
    /* p. 107: n = 10, beta = 1.618, input 5 10 7 21 13 8 2, linear probing */
    int in[] = { 5, 10, 7, 21, 13, 8, 2 };
    int want_hash[] = { 0, 1, 3, 9, 0, 9, 2 };
    open_table_t t;
    for (int i = 0; i < 7; i++) CHECK(hash_golden(in[i], 10) == want_hash[i]);
    /* the slide lists h(13) = 3, but 13 * 1.618 = 21.034, so h(13) = 0 */

    /* with the slide's hash values the table is the slide's table */
    int slide_hash[] = { 0, 1, 3, 9, 3, 9, 2 };
    int slide_slot[] = { 0, 1, 3, 9, 4, 2, 5 };
    ot_init(&t);
    for (int i = 0; i < 7; i++) CHECK(ot_insert(&t, in[i], slide_hash[i]) == slide_slot[i]);

    /* with the correct values, 13 lands in slot 2 and 8 in slot 4 */
    int good_slot[] = { 0, 1, 3, 9, 2, 4, 5 };
    ot_init(&t);
    for (int i = 0; i < 7; i++) CHECK(ot_insert(&t, in[i], hash_golden(in[i], 10)) == good_slot[i]);

    /* delete 7 (slot 3): it becomes "deleted", not "empty", so 8 (hash 9,
       probes 9 0 1 2 3 4) is still found behind it */
    CHECK(ot_delete(&t, 7, hash_golden(7, 10)));
    CHECK(t.state[3] == SLOT_DELETED);
    CHECK(ot_find(&t, 8, hash_golden(8, 10)) == 4);
    CHECK(ot_find(&t, 7, hash_golden(7, 10)) == -1);
    CHECK(ot_insert(&t, 33, 3) == 3);                         /* a deleted slot is reused */
}

TEST(hash_table_chaining)
{
    chained_table_t t;
    ct_init(&t);
    for (int k = 0; k < 100; k += 3) ct_insert(&t, k);
    int ok = 1;
    for (int k = 0; k < 100; k++) ok &= ct_contains(&t, k) == (k % 3 == 0);
    CHECK(ok);
    ct_free(&t);
}

TEST(binary_search_tree)
{
    tnode_t *root = NULL;
    int keys[] = { 10, 5, 15, 3, 7, 12, 18, 6, 8 }, out[16];
    for (int i = 0; i < 9; i++) root = bst_insert(root, keys[i]);
    root = bst_insert(root, 7);                               /* duplicate ignored */
    CHECK(bst_inorder(root, out, 0) == 9 && sorted(out, 9));
    CHECK(bst_preorder(root, out, 0) == 9 && out[0] == 10 && out[1] == 5 && out[2] == 3);
    CHECK(bst_postorder(root, out, 0) == 9 && out[0] == 3 && out[8] == 10);
    CHECK(bst_breadth_first(root, out) == 9 && memcmp(out, (int[]){ 10, 5, 15, 3, 7, 12, 18, 6, 8 }, sizeof(int) * 9) == 0);
    CHECK(bst_next(bst_find(root, 8))->key == 10);            /* up to the first ancestor from the left */
    CHECK(bst_next(bst_find(root, 5))->key == 6);             /* minimum of the right subtree */
    CHECK(bst_next(bst_find(root, 18)) == NULL);
    root = bst_delete(root, 5);                               /* two children */
    root = bst_delete(root, 3);                               /* leaf */
    root = bst_delete(root, 10);                              /* the root */
    CHECK(bst_inorder(root, out, 0) == 6 && memcmp(out, (int[]){ 6, 7, 8, 12, 15, 18 }, sizeof(int) * 6) == 0);
    CHECK(bst_find(root, 5) == NULL && root->parent == NULL);
    bst_free(root);
}

TEST(avl_stays_balanced)
{
    /* 1..1023 in increasing order: a plain BST becomes a 1023-long chain,
       the AVL tree stays at height 10 */
    anode_t *t = NULL;
    tnode_t *plain = NULL;
    int rotations = 0, out[1023];
    for (int i = 1; i <= 1023; i++) { t = avl_insert(t, i, &rotations); if (i <= 200) plain = bst_insert(plain, i); }
    CHECK(avl_is_balanced(t));
    CHECK(avl_height(t) == 10);
    CHECK(bst_height(plain) == 200);
    CHECK(avl_inorder(t, out, 0) == 1023 && sorted(out, 1023));
    CHECK(rotations > 0);
    anode_t *d = NULL; rotations = 0;                         /* a double rotation: 30, 10, 20 */
    d = avl_insert(d, 30, &rotations); d = avl_insert(d, 10, &rotations); d = avl_insert(d, 20, &rotations);
    CHECK(rotations == 2 && d->key == 20 && avl_is_balanced(d));
    avl_free(t); avl_free(d); bst_free(plain);
}

TEST(graph_algorithms)
{
    graph_t g; int order[G_MAX], dist[G_MAX];
    graph_init(&g, 6);
    graph_add_edge(&g, 0, 1, 7); graph_add_edge(&g, 0, 2, 9); graph_add_edge(&g, 0, 5, 14);
    graph_add_edge(&g, 1, 2, 10); graph_add_edge(&g, 1, 3, 15); graph_add_edge(&g, 2, 3, 11);
    graph_add_edge(&g, 2, 5, 2); graph_add_edge(&g, 3, 4, 6); graph_add_edge(&g, 4, 5, 9);
    CHECK(graph_bfs(&g, 0, order) == 6 && memcmp(order, (int[]){ 0, 1, 2, 5, 3, 4 }, sizeof(int) * 6) == 0);
    CHECK(graph_dfs(&g, 0, order) == 6 && memcmp(order, (int[]){ 0, 1, 2, 3, 4, 5 }, sizeof(int) * 6) == 0);
    graph_dijkstra(&g, 0, dist);
    CHECK(dist[4] == 20 && dist[3] == 20 && dist[5] == 11);   /* 0-2-5-4 = 9+2+9 */
    CHECK(!graph_has_euler_circuit(&g));                      /* odd degrees */
    graph_t sq; graph_init(&sq, 4);
    graph_add_edge(&sq, 0, 1, 1); graph_add_edge(&sq, 1, 2, 1); graph_add_edge(&sq, 2, 3, 1); graph_add_edge(&sq, 3, 0, 1);
    CHECK(graph_has_euler_circuit(&sq) && graph_is_bipartite(&sq));   /* a 4-cycle */
    graph_add_edge(&sq, 0, 2, 1);
    CHECK(!graph_is_bipartite(&sq));                          /* a triangle needs 3 colours */
}

int main(void)
{
    RUN(analysis_min_max_horner_power);
    RUN(linked_list_operations);
    RUN(self_organising_list);
    RUN(stack_applications);
    RUN(cyclic_array_queue);
    RUN(sparse_matrix);
    RUN(sorts_agree_with_qsort);
    RUN(sort_costs_match_the_theory);
    RUN(binary_search_steps);
    RUN(heap_priority_queue);
    RUN(hash_table_lecture_example);
    RUN(hash_table_chaining);
    RUN(binary_search_tree);
    RUN(avl_stays_balanced);
    RUN(graph_algorithms);
    return REPORT();
}
