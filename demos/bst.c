#include <stdlib.h>
#include "demos.h"

tnode_t *bst_insert(tnode_t *root, int key)
{
    tnode_t *parent = NULL, **link = &root;
    while (*link) {
        parent = *link;
        if (key == parent->key) return root;            /* no duplicates */
        link = key < parent->key ? &parent->left : &parent->right;
    }
    tnode_t *n = calloc(1, sizeof *n);
    if (!n) return root;
    n->key = key; n->parent = parent;
    *link = n;
    return root;
}

tnode_t *bst_find(tnode_t *t, int key)
{
    while (t && t->key != key) t = key < t->key ? t->left : t->right;
    return t;
}

tnode_t *bst_min(tnode_t *t) { while (t && t->left) t = t->left; return t; }

tnode_t *bst_next(tnode_t *t)
{
    /* p. 145: the minimum of the right subtree, or the first ancestor we
       reach from its left side */
    if (t->right) return bst_min(t->right);
    tnode_t *p = t->parent;
    while (p && t == p->right) { t = p; p = p->parent; }
    return p;
}

static void replace_child(tnode_t **root, tnode_t *old, tnode_t *new_)
{
    if (old->parent == NULL) *root = new_;
    else if (old->parent->left == old) old->parent->left = new_;
    else old->parent->right = new_;
    if (new_) new_->parent = old->parent;
}

tnode_t *bst_delete(tnode_t *root, int key)
{
    tnode_t *z = bst_find(root, key);
    if (!z) return root;
    if (z->left && z->right) {                          /* two children: copy the successor */
        tnode_t *s = bst_min(z->right);
        z->key = s->key;
        z = s;                                          /* s has no left child */
    }
    tnode_t *child = z->left ? z->left : z->right;
    replace_child(&root, z, child);
    free(z);
    return root;
}

size_t bst_inorder(const tnode_t *t, int *out, size_t i)
{
    if (!t) return i;
    i = bst_inorder(t->left, out, i);
    out[i++] = t->key;
    return bst_inorder(t->right, out, i);
}

size_t bst_preorder(const tnode_t *t, int *out, size_t i)
{
    if (!t) return i;
    out[i++] = t->key;
    i = bst_preorder(t->left, out, i);
    return bst_preorder(t->right, out, i);
}

size_t bst_postorder(const tnode_t *t, int *out, size_t i)
{
    if (!t) return i;
    i = bst_postorder(t->left, out, i);
    i = bst_postorder(t->right, out, i);
    out[i++] = t->key;
    return i;
}

size_t bst_breadth_first(tnode_t *root, int *out)
{
    /* p. 161: a queue of nodes, level by level */
    tnode_t *queue[256];
    size_t head = 0, tail = 0, n = 0;
    if (root) queue[tail++] = root;
    while (head < tail) {
        tnode_t *t = queue[head++];
        out[n++] = t->key;
        if (t->left && tail < 256) queue[tail++] = t->left;
        if (t->right && tail < 256) queue[tail++] = t->right;
    }
    return n;
}

int bst_height(const tnode_t *t)
{
    if (!t) return 0;
    int l = bst_height(t->left), r = bst_height(t->right);
    return 1 + (l > r ? l : r);
}

void bst_free(tnode_t *t) { if (t) { bst_free(t->left); bst_free(t->right); free(t); } }
