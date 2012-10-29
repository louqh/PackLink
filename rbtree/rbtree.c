#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "rbtree.h"

static struct rbtree_node *NIL = NULL;

static void rbtree_addrow(struct rbtree *r)
{
    unsigned u;
    if (&ROW(r, r->length) >= r->array + r->rows) {
        u = r->rows * 2;
        r->array = realloc(r->array, sizeof(*r->array) * u);
        assert(r->array);
        while (r->rows < u)
            r->array[r->rows++] = NULL;
    }
    assert(ROW(r, r->length) == NULL);
    ROW(r, r->length) = malloc(sizeof(**r->array) * ROW_WIDTH);
    assert(ROW(r, r->length));
    r->length += ROW_WIDTH;
}

struct rbtree *rbtree_new(void)
{
    struct rbtree *r;
    unsigned u;

    r = calloc(1, sizeof *r);
    if (!r)
        return NULL;

    r->page_size = (unsigned)getpagesize() / sizeof(void *);
    r->page_mask = r->page_size - 1;
    assert((r->page_size & r->page_mask) == 0);
    for (u = 1; (1U << u) != r->page_size; u++)
        ;
    r->page_shift = u;
    assert(r->page_size <= (sizeof(**r->array) * ROW_WIDTH));

    r->next = 1;
    r->root = 0;
    r->rows = 16;
    r->array = calloc(r->rows, sizeof *r->array);
    assert(r->array);
    rbtree_addrow(r);
    NIL = calloc(1, sizeof *NIL);
    NIL->color = BLACK;
    A(r, r->root) = NIL;
    return r;
}

void rbtree_free(struct rbtree *t)
{
    assert(t != NULL);
    unsigned i;
    for (i = 0; i < t->next; i++) {
        struct rbtree_node *n = A(t, i);
        free(n->ptr);
        free(A(t, i));
        A(t, i) = NULL;
    }
    free(*t->array);
    free(t->array);
    free(t);
}

static void rbtree_left_rotate(struct rbtree *t, unsigned u)
{
    struct rbtree_node *x, *y;
    x = A(t, u);
    unsigned v = x->children[1];
    y = A(t, v);
    x->children[1] = y->children[0];

    if (y->children[0]) {
        struct rbtree_node *p = A(t, y->children[0]);
        p->parent = u;
    }

    y->parent = x->parent;
    struct rbtree_node *xp = A(t, x->parent);
    if (xp == NIL)
        t->root = v;
    else if (u == xp->children[0])
        xp->children[0] = v;
    else
        xp->children[1] = v;
    x->parent = v;
    y->children[0] = u;
}

static void rbtree_right_rotate(struct rbtree *t, unsigned u)
{
    struct rbtree_node *x, *y;
    x = A(t, u);
    unsigned v = x->children[0];
    y = A(t, v);
    x->children[0] = y->children[1];

    if (y->children[1]) {
        struct rbtree_node *p = A(t, y->children[1]);
        p->parent = u;
    }

    y->parent = x->parent;
    struct rbtree_node *xp = A(t, x->parent);
    if (xp == NIL)
        t->root = v;
    else if (u == xp->children[0])
        xp->children[0] = v;
    else
        xp->children[1] = v;
    x->parent = v;
    y->children[1] = u;
}

static unsigned rbtree_grandparent(struct rbtree *t, unsigned u)
{
    struct rbtree_node *x = A(t, u);
    if (u && x->parent) {
        struct rbtree_node *xx = A(t, x->parent);
        return xx->parent;
    }
    return 0;
}

static void rbtree_insert_fixup(struct rbtree *t, unsigned u)
{
    struct rbtree_node *y;
    struct rbtree_node *x = A(t, u);
    struct rbtree_node *xp = A(t, x->parent);
    while (xp->color == RED) {
        unsigned g = rbtree_grandparent(t, u);
        struct rbtree_node *gp = A(t, g);
        if (x->parent == gp->children[0]) {
            y = A(t, gp->children[1]);
            if (y->color == RED) {
                xp->color = BLACK;
                y->color = BLACK;
                gp->color = RED;
                u = rbtree_grandparent(t, u);
            } else {
                if (u == xp->children[1]) {
                    u = x->parent;
                    rbtree_left_rotate(t, u);
                }
                x = A(t, u);
                xp = A(t, x->parent);
                xp->color = BLACK;
                g = rbtree_grandparent(t, u);
                gp = A(t, g);
                gp->color = RED;
                rbtree_right_rotate(t, g);
            }
        } else {
            y = A(t, gp->children[0]);
            if (y->color == RED) {
                xp->color = BLACK;
                y->color = BLACK;
                gp->color = RED;
                u = rbtree_grandparent(t, u);
            } else {
                if (u == xp->children[0]) {
                    u = x->parent;
                    rbtree_right_rotate(t, u);
                }
                x = A(t, u);
                xp = A(t, x->parent);
                xp->color = BLACK;
                g = rbtree_grandparent(t, u);
                gp->color = RED;
                rbtree_left_rotate(t, g);
            }
        }
        x = A(t, u);
        xp = A(t, x->parent);
    }
    x = A(t, t->root);
    x->color = BLACK;
}

void rbtree_insert(struct rbtree *t, struct rbtree_node *n)
{
    unsigned u;
    struct rbtree_node *xn;
    assert(t);
    assert(t->length >= t->next);
    if (t->length == t->next)
        rbtree_addrow(t);
    assert(t->length > t->next);

    unsigned x, y;
    x = t->root;
    xn = A(t, x);
    y = 0;

    while (xn != NIL) {
        y = x;
        if (n->key < xn->key)
            x = xn->children[0];
        else
            x = xn->children[1];
        xn = A(t, x);
    }

    n->parent = y;
    n->children[0] = n->children[1] = 0;
    n->color = RED;

    u = t->next++;
    A(t, u) = n;

    struct rbtree_node *yp = A(t, y);

    if (yp == NIL)
        t->root = u;
    else if (n->key < yp->key)
        yp->children[0] = u;
    else
        yp->children[1] = u;
    rbtree_insert_fixup(t, u);
}
