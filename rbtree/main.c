#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "rbtree.h"

static void dot_export(struct rbtree *t, FILE *fp);
static void dot_print_node(struct rbtree *t, unsigned u, FILE *fp);
static unsigned long jenkins(const void *key);
static int parse_args(int argc, char **argv);

static char *export = NULL;
static char *infile = NULL;

int main(int argc, char **argv)
{
    struct rbtree *t;
    struct rbtree_node **nodes;
    int c, i = 0;
    char buf[256];

    if (parse_args(argc, argv) == -1)
        exit(1);

    FILE *fp = NULL;
    if (!infile) {
        fp = stdin;
    } else {
        fp = fopen(infile, "r");
        if (!fp) {
            fprintf(stderr, "Can't open file: %s\n", infile);
            exit(1);
        }
    }

    t = rbtree_new();
    unsigned N = 100;
    unsigned next = 0;
    nodes = calloc(100, sizeof *nodes);

    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') {
            i = 0;
            if (next >= N) {
                nodes = realloc(nodes, N*2 * sizeof *nodes);
                N *= 2;
            }
            nodes[next] = calloc(1, sizeof *nodes[next]);
            nodes[next]->key = jenkins(buf);
            nodes[next]->ptr = strdup(buf);
            rbtree_insert(t, nodes[next]);
            next++;
            memset(buf, 0, sizeof buf);
        } else {
            buf[i++] = c;
        }
    }

    if (infile)
        fclose(fp);

    if (export) {
        fp = fopen(export, "w");
        dot_export(t, fp);
        fclose(fp);
    }

    free(nodes);
    rbtree_free(t);
    if (infile)
        free(infile);
    if (export)
        free(export);
    return 0;
}

static void dot_export(struct rbtree *t, FILE *fp)
{
    fprintf(fp, "digraph RBTree {\n");
    fprintf(fp, "    node [style=filled, fontname=\"Arial\"];\n");
    dot_print_node(t, t->root, fp);
    fprintf(fp, "}\n");
}

static void dot_print_node(struct rbtree *t, unsigned u, FILE *fp)
{
    struct rbtree_node *n = A(t, u);
    char *s = n->color == BLACK ? "grey" : "white";

    if (n->children[0]) {
        struct rbtree_node *l = A(t, n->children[0]);
        fprintf(fp, "    %lu [fillcolor=%s];\n", n->key, s);
        fprintf(fp, "    %lu -> %lu;\n", n->key, l->key);
        dot_print_node(t, n->children[0], fp);
    } else {
        fprintf(fp, "    %lu [fillcolor=%s];\n", n->key, s);
    }

    if (n->children[1]) {
        struct rbtree_node *r = A(t, n->children[1]);
        char *s = n->color == BLACK ? "grey" : "white";
        fprintf(fp, "    %lu [fillcolor=%s];\n", n->key, s);
        fprintf(fp, "    %lu -> %lu;\n", n->key, r->key);
        dot_print_node(t, n->children[1], fp);
    } else {
        fprintf(fp, "    %lu [fillcolor=%s];\n", n->key, s);
    }
}

static unsigned long jenkins(const void *key)
{
    unsigned long hash, i;
    const char *k = (const char *)key;
    size_t len = strlen(k);
    for (hash = i = 0; i < len; i++) {
        hash += k[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

static int parse_args(int argc, char **argv)
{
    static struct option opts[] = {
        {"help",   no_argument,       0, 'h'},
        {"export", required_argument, 0, 'e'},
        {0, 0, 0, 0}
    };

    while (1) {
        int c, i = 0;
        c = getopt_long(argc, argv, "he:", opts, &i);
        if (c == -1)
            break;

        switch (c) {
        case 'h':
            return -1;
        case 'e':
            export = strdup(optarg);
            break;
        default:
            return -1;
        }
    }

    if (argv[optind])
        infile = strdup(argv[optind]);
    return 0;
}
