/*
 * rbtree.c - red-black tree (pp. 179-181).
 *
 * The slides define the tree by its colour rules: the root is black, the
 * empty leaves are black, a red node has no red child, and every path from
 * the root to a leaf passes the same number of black nodes. From these the
 * height is at most 2 log2 n (p. 181).
 *
 * Insertion here is the left-leaning variant (Sedgewick): a red node is
 * always a left child, which leaves three small repair steps on the way back
 * up. Every tree it builds obeys the rules above; it is a subset of the
 * red-black trees, chosen because the code is short enough to check by hand.
 * The empty leaves are NULL pointers.
 */
#include <limits.h>
#include <stdlib.h>
#include "demos.h"

static bool is_red(const rbnode_t *t) { return t && t->red; }

static rbnode_t *rotate_left(rbnode_t *h)
{
    rbnode_t *x = h->right;
    h->right = x->left; x->left = h;
    x->red = h->red; h->red = true;                 /* the link that leans stays red */
    return x;
}

static rbnode_t *rotate_right(rbnode_t *h)
{
    rbnode_t *x = h->left;
    h->left = x->right; x->right = h;
    x->red = h->red; h->red = true;
    return x;
}

static rbnode_t *insert(rbnode_t *h, int key, int *rotations, int *recolourings)
{
    if (!h) {
        rbnode_t *n = calloc(1, sizeof *n);
        if (n) { n->key = key; n->red = true; }     /* a new node is red: black heights unchanged */
        return n;
    }
    if (key < h->key) h->left = insert(h->left, key, rotations, recolourings);
    else if (key > h->key) h->right = insert(h->right, key, rotations, recolourings);
    else return h;

    if (is_red(h->right) && !is_red(h->left)) { h = rotate_left(h); (*rotations)++; }         /* red leans right */
    if (is_red(h->left) && is_red(h->left->left)) { h = rotate_right(h); (*rotations)++; }    /* two reds in a row */
    if (is_red(h->left) && is_red(h->right)) {      /* both children red: push the red up one level */
        h->red = true; h->left->red = false; h->right->red = false;
        (*recolourings)++;
    }
    return h;
}

rbnode_t *rb_insert(rbnode_t *root, int key, int *rotations, int *recolourings)
{
    root = insert(root, key, rotations, recolourings);
    if (root) root->red = false;                    /* rule 2: the root is black */
    return root;
}

bool rb_contains(const rbnode_t *t, int key)
{
    while (t && t->key != key) t = key < t->key ? t->left : t->right;
    return t != NULL;
}

int rb_black_height(const rbnode_t *t)
{
    if (!t) return 1;                               /* the empty leaf counts as one black node */
    int l = rb_black_height(t->left), r = rb_black_height(t->right);
    if (l < 0 || r < 0 || l != r) return -1;
    return l + (t->red ? 0 : 1);
}

static bool ordered_no_red_red(const rbnode_t *t, long lo, long hi)
{
    if (!t) return true;
    if (t->key <= lo || t->key >= hi) return false;
    if (t->red && (is_red(t->left) || is_red(t->right))) return false;
    return ordered_no_red_red(t->left, lo, t->key) && ordered_no_red_red(t->right, t->key, hi);
}

bool rb_is_valid(const rbnode_t *root)
{
    if (is_red(root)) return false;
    return ordered_no_red_red(root, (long)INT_MIN - 1, (long)INT_MAX + 1) && rb_black_height(root) > 0;
}

int rb_height(const rbnode_t *t)
{
    if (!t) return 0;
    int l = rb_height(t->left), r = rb_height(t->right);
    return 1 + (l > r ? l : r);
}

size_t rb_inorder(const rbnode_t *t, int *out, size_t i)
{
    if (!t) return i;
    i = rb_inorder(t->left, out, i);
    out[i++] = t->key;
    return rb_inorder(t->right, out, i);
}

void rb_free(rbnode_t *t) { if (t) { rb_free(t->left); rb_free(t->right); free(t); } }
