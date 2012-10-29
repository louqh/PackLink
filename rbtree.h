#ifndef RBTREE_H
#define RBTREE_H

#define RED   0
#define BLACK 1

#define ROW_SHIFT 16
#define ROW_WIDTH (1 << ROW_SHIFT)

#define ROW(b, n) ((b)->array[(n) >> ROW_SHIFT])
#define A(b, n)   ROW(b, n)[(n) & (ROW_WIDTH - 1)]

struct rbtree_node {
    char color;
    unsigned long key;
    void *ptr;
    unsigned parent;
    unsigned children[2];
};

struct rbtree {
    void ***array;
    unsigned root;
    unsigned rows;
    unsigned length;
    unsigned next;
    unsigned page_size;
    unsigned page_mask;
    unsigned page_shift;
};

struct rbtree *rbtree_new(void);
void rbtree_free(struct rbtree *t);
void rbtree_insert(struct rbtree *t, struct rbtree_node *n);

#endif
