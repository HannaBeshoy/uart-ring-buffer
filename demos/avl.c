#include <stdlib.h>
#include "demos.h"

int avl_height(const anode_t *t) { return t ? t->height : 0; }
static int max2(int a, int b) { return a > b ? a : b; }
static void fix(anode_t *t) { t->height = 1 + max2(avl_height(t->left), avl_height(t->right)); }
static int balance(const anode_t *t) { return avl_height(t->right) - avl_height(t->left); }

static anode_t *rotate_left(anode_t *x)
{
    anode_t *y = x->right;
    x->right = y->left; y->left = x;
    fix(x); fix(y);
    return y;
}

static anode_t *rotate_right(anode_t *y)
{
    anode_t *x = y->left;
    y->left = x->right; x->right = y;
    fix(y); fix(x);
    return x;
}

anode_t *avl_insert(anode_t *t, int key, int *rotations)
{
    if (!t) {
        anode_t *n = calloc(1, sizeof *n);
        if (n) { n->key = key; n->height = 1; }
        return n;
    }
    if (key < t->key) t->left = avl_insert(t->left, key, rotations);
    else if (key > t->key) t->right = avl_insert(t->right, key, rotations);
    else return t;
    fix(t);
    int b = balance(t);
    if (b < -1) {                                   /* left heavy (case (-2, .)) */
        if (balance(t->left) > 0) { t->left = rotate_left(t->left); (*rotations)++; }   /* double */
        (*rotations)++;
        return rotate_right(t);
    }
    if (b > 1) {                                    /* right heavy (case (2, .), p. 168) */
        if (balance(t->right) < 0) { t->right = rotate_right(t->right); (*rotations)++; }
        (*rotations)++;
        return rotate_left(t);
    }
    return t;
}

/* Restore the balance at t after a deletion below it (pp. 177-178): single
   rotation in cases (2, 1) and (2, 0), double rotation in case (2, -1), and
   the mirror images. The subtree may come back one level lower, so the
   caller, one level up, checks its own balance again. */
static anode_t *rebalance_after_delete(anode_t *t, int *rotations)
{
    fix(t);
    int b = balance(t);
    if (b < -1) {
        if (balance(t->left) > 0) { t->left = rotate_left(t->left); (*rotations)++; }
        (*rotations)++;
        return rotate_right(t);
    }
    if (b > 1) {
        if (balance(t->right) < 0) { t->right = rotate_right(t->right); (*rotations)++; }
        (*rotations)++;
        return rotate_left(t);
    }
    return t;
}

anode_t *avl_delete(anode_t *t, int key, int *rotations)
{
    if (!t) return NULL;                            /* not in the tree: nothing to do */
    if (key < t->key) t->left = avl_delete(t->left, key, rotations);
    else if (key > t->key) t->right = avl_delete(t->right, key, rotations);
    else if (!t->left || !t->right) {               /* no child or one child: lift the child */
        anode_t *child = t->left ? t->left : t->right;
        free(t);
        return child;                               /* already balanced, heights unchanged */
    } else {                                        /* two children: take the in-order successor's key */
        anode_t *next = t->right;
        while (next->left) next = next->left;
        t->key = next->key;
        t->right = avl_delete(t->right, next->key, rotations);
    }
    return rebalance_after_delete(t, rotations);
}

bool avl_contains(const anode_t *t, int key)
{
    while (t && t->key != key) t = key < t->key ? t->left : t->right;
    return t != NULL;
}

bool avl_is_balanced(const anode_t *t)
{
    if (!t) return true;
    int b = balance(t);
    return b >= -1 && b <= 1 && t->height == 1 + max2(avl_height(t->left), avl_height(t->right))
           && avl_is_balanced(t->left) && avl_is_balanced(t->right);
}

size_t avl_inorder(const anode_t *t, int *out, size_t i)
{
    if (!t) return i;
    i = avl_inorder(t->left, out, i);
    out[i++] = t->key;
    return avl_inorder(t->right, out, i);
}

void avl_free(anode_t *t) { if (t) { avl_free(t->left); avl_free(t->right); free(t); } }
