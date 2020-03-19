#ifndef RB_TREE_H
#define RB_TREE_H

#include <stdbool.h>

struct rb_node;

typedef int (*rb_cmp)(void *, void *);

typedef void (*rb_free)(void *);

typedef void (*rb_print)(void *);

struct rb_tree{
  struct rb_node * root;
  rb_cmp cmp;
  rb_free fr;
};

void rb_tree_init(struct rb_tree * tree, rb_cmp cmp, rb_free fr);

void rb_tree_insert(struct rb_tree * tree, void * value);

bool rb_tree_delete(struct rb_tree * tree, void * value);

void rb_tree_print(const struct rb_tree * tree, rb_print pr);

void rb_tree_dispose(struct rb_tree * tree);

#endif
