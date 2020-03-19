#include "rb_tree.h"

#include <stdio.h>

static int cmp_element(void * value1, void * value2){
  return (*(int *)value1) - (*(int *)value2);
}

static void print_element(void * value){
  printf("%d",*((int*)value));
}

int main(int arg_count, const char ** args){

  int numbers[] = {7,2,1,8,9,5,3,4,6,0};

  struct rb_tree tree;

  rb_tree_init(&tree, &cmp_element, NULL);

  for(int i = 0; i < 10; ++i){
    rb_tree_insert(&tree, &numbers[i]);
    printf("tree after insertion of %d:\n", numbers[i]);
    rb_tree_print(&tree, &print_element);
  }
  for(int i = 5; i < 15; ++i){
    rb_tree_delete(&tree, &numbers[i % 10]);
    printf("tree after deletion of %d\n", numbers[i % 10]);
    //rb_tree_print(&tree, &print_element);
  }
  
  return 0;
}
