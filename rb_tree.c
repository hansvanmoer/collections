#include "memory.h"
#include "rb_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct rb_node{
  struct rb_node * parent;
  struct rb_node * left;
  struct rb_node * right;
  bool red;
  
  void * value;
};


/*
 * Helper functions
 */

static struct rb_node * get_head(struct rb_node * root){
  if(root == NULL){
    return NULL;
  }else{
    struct rb_node * node = root;
    while(node->left != NULL){
      node = node->left;
    }
    return node;
  }
}

static struct rb_node * get_next(struct rb_node * node){
  assert(node != NULL);

  if(node->right == NULL){
    //walk upwards until we end up in the left subtree or at the root
    while(node->parent != NULL && node->parent->right == node){
      node = node->parent;
    }
    return node->parent;
  }else{
    //find lowest node in right subtree
    node = node->right;
    while(node->left != NULL){
      node = node->left;
    }
    return node;
  }
}

static struct rb_node * create_node(void * value){
  struct rb_node * node = malloc_checked(sizeof(struct rb_node));
  node->value = value;
  node->red = true;
  node->left = NULL;
  node->right = NULL;
  return node;
}

static void free_value(struct rb_tree * tree, void * value){
  if(tree->fr != NULL){
    (*tree->fr)(value);
  }
}

static void free_node(struct rb_tree * tree, struct rb_node * node){
  free_value(tree, node->value);
  free(node);
}

static struct rb_node * find(struct rb_tree * tree, struct rb_node * pos, void * value){
  while(pos != NULL){
    int cmp = (*tree->cmp)(value, pos->value);
    if(cmp < 0){
      pos = pos->left;
    }else if(cmp > 0){
      pos = pos->right;
    }else{
      return pos;
    }
  }
  return pos;
}

/*
 * Initialization functions
 */

void rb_tree_init(struct rb_tree * tree, rb_cmp cmp, rb_free fr){
  assert(tree != NULL);
  assert(cmp != NULL);
  tree->root = NULL;
  tree->cmp = cmp;
  tree->fr = fr;
}

/*
 * Print functions
 */

static void print_node(struct rb_node * node, rb_print pr){
  assert(node != NULL);
  assert(pr != NULL);
  
  printf("node ");
  (*pr)(node->value);
  printf("(%s)",node->red ? "red  " : "black");
  if(node->parent){
    printf(" parent ");
    (*pr)(node->parent->value);
  }else{
    printf(" root    ");
  }
  if(node->left){
    printf(" left ");
    (*pr)(node->left->value);
  }else{
    printf(" left n");
  }
  if(node->right){
    printf(" right ");
    (*pr)(node->right->value);
    printf("\n");
  }else{
    printf(" right n\n");
  }
  
}

void rb_tree_print(const struct rb_tree * tree, rb_print pr){
  assert(tree != NULL);
  assert(pr != NULL);
  struct rb_node * pos = get_head(tree->root);
  while(pos != NULL){
    print_node(pos, pr);
    pos = get_next(pos);
  }
}

/*
 * Insert functions
 */

static struct rb_node * get_uncle(struct rb_node * node){
  assert(node != NULL);
  assert(node->parent != NULL);
  assert(node->parent->parent != NULL);
  
  if(node->parent == node->parent->parent->left){
    return node->parent->parent->right;
  }else{
    return node->parent->parent->left;
  }
}

static void rotate_right(struct rb_tree * tree, struct rb_node * pivot){
  assert(tree != NULL);
  assert(pivot != NULL);
  assert(pivot->parent != NULL);
  assert(pivot == pivot->parent->left);

  struct rb_node * parent = pivot->parent;
  struct rb_node * child = pivot->right;

  parent->left = child;
  if(child != NULL){
    child->parent = parent;
  }

  pivot->parent = parent->parent;
  if(parent->parent == NULL){
    tree->root = pivot;
  }else{
    if(parent == parent->parent->left){
      parent->parent->left = pivot;
    }else{
      parent->parent->right = pivot;
    }
  }
  
  pivot->right = parent;
  parent->parent = pivot;
}

static void rotate_left(struct rb_tree * tree, struct rb_node * pivot){
  assert(tree != NULL);
  assert(pivot != NULL);
  assert(pivot->parent != NULL);
  assert(pivot == pivot->parent->right);

  struct rb_node * parent = pivot->parent;
  struct rb_node * child = pivot->left;

  parent->right = child;
  if(child != NULL){
    child->parent = parent;
  }

  pivot->parent = parent->parent;
  if(parent->parent == NULL){
    tree->root = pivot;
  }else{
    if(parent == parent->parent->left){
      parent->parent->left = pivot;
    }else{
      parent->parent->right = pivot;
    }
  }
  
  pivot->left = parent;
  parent->parent = pivot;
}

static void fix_after_insert(struct rb_tree * tree, struct rb_node * node){
  assert(tree != NULL);
  assert(node != NULL);

  while(true){
    if(node->parent == NULL){
      //node is root => paint black and it is done
      node->red = false;
      break;
    }else if(!node->parent->red){
      //parent is black => nothing needs to be done
      break;
    }else{
      //note that parent can not be root because it is red
      struct rb_node * uncle = get_uncle(node);
      if(uncle != NULL && uncle->red){
	//parent is red and uncle is red => repaint some nodes and move up the tree
	uncle->red = false;
	node->parent->red = false;
	node = node->parent->parent;
	node->red = true;
      }else{
	//parent is red and uncle is black => do some rotations
	//first: ensure that the node is not on the inside of a subtree by rotating as needed
 	if(node == node->parent->right && node->parent == node->parent->parent->left){
	  rotate_left(tree, node);
	  node = node->left; 
	}else if(node == node->parent->left && node->parent == node->parent->parent->right){
	  rotate_right(tree, node);
	  node = node->right; 
	}

	//rotate the parent node so that it becomes the root of the subtree
	//then paint both intitial parent and grandparents
	if(node == node->parent->left){
	  rotate_right(tree, node->parent);
	  node->parent->right->red =true;
	}else{
	  rotate_left(tree, node->parent);
	  node->parent->left->red = true;
	}
	node->parent->red = false;
	break;
      }
    }
  }
}

void rb_tree_insert(struct rb_tree * tree, void * value){
  assert(tree != NULL);

  if(tree->root == NULL){
    struct rb_node * node = create_node(value);
    tree->root = node;
    node->parent = NULL;
    fix_after_insert(tree, node);
  }else{
    struct rb_node * pos = tree->root;
    while(true){
      int cmp = (*tree->cmp)(value, pos->value);
      if(cmp < 0){
	if(pos->left == NULL){
	  struct rb_node * node = create_node(value);
	  node->parent = pos;
	  pos->left = node;
	  fix_after_insert(tree, node);
	  break;
	}else{
	  pos = pos->left;
	}
      }else if(cmp > 0){
	if(pos->right == NULL){
	  struct rb_node * node = create_node(value);
	  node->parent = pos;
	  pos->right = node;
	  fix_after_insert(tree, node);
	  break;
	}else{
	  pos = pos->right;
	}
      }else{
        free_value(tree, pos->value);
	pos->value = value;
	break;
      }
    }
  }
}

/*
 * Delete functions
 */

static void replace_with_child(struct rb_tree * tree, struct rb_node * node, struct rb_node * child){
  assert(node != NULL);
  assert(child != NULL);
  
  if(node->parent == NULL){
    tree->root = child;
    child->parent = NULL;
  }else{
    if(node->parent->left == node){
      node->parent->left = child;
    }else{
      node->parent->right = child;
    }
    child->parent = node->parent;
  }
}

static void detach_node(struct rb_tree * tree, struct rb_node * node){
  assert(tree != NULL);
  assert(node != NULL);

  if(node->parent == NULL){
    tree->root = NULL;
  }else{
    if(node->parent->left == node){
      node->parent->left = NULL;
    }else{
      node->parent->right = NULL;
    }
  }
}

static struct rb_node * pick_node_to_delete(struct rb_node * node){
  assert(node != NULL);
 //does the node have at most one non leaf child?
  if(node->right != NULL && node->left != NULL){
    //No: find such a node (the succesor node) and swap values
    struct rb_node * n = node->right;
    while(n->left != NULL){
      n = n->left;
    }
    void * tmp_value = n->value;
    n->value = node->value;
    node->value = tmp_value;
    node = n;
  }
  return node;
}

static void delete_node(struct rb_tree * tree, struct rb_node * node){
  assert(tree != NULL);
  assert(node != NULL);
 
  //swap node if needed
  node = pick_node_to_delete(node);
  
  // node now has at most one non-leaf child
  assert(node->left == NULL || node->right == NULL);

  //child is non leaf child if such a child exists
  struct rb_node * child;
  if(node->left != NULL){
    child = node->left;
  }else if(node->right != NULL){
    child = node->right;
  }else{
    child = NULL;
  }

  if(node->red){
    //node is red, child is therefore black => replace node with its child.
    if(child == NULL){
      detach_node(tree, node);
    }else{
      replace_with_child(tree, node, child);
    }
  }else if(child != NULL && child->red){
    //node is black and child is red => replace node with child and repaint child to preserve # of black nodes
    replace_with_child(tree, node, child);
    child->ref = false;
  }else{ //note: child has to be a (black) leaf of a black parent
    //we replace node with its leaf child (NULL), but only at the end as setting it to NULL complicates things.
    //for now: assume node is a leaf and is black
    if(node->parent == NULL){
      //the node to remove is the root node
      //nothing has to be done.
      tree->root = NULL;
      free_node(tree, node);
    }else{
      if(node == node->parent->left){
	struct rb_node * sibling = node->parent->right;
	if(sibling != NULL && sibling->red){
	  //sibling is red => paint sibling and parent
	  sibling->red = false;
	  node->parent->red = true;
	}
      }else{

      }
    }
  }
}

bool rb_tree_delete(struct rb_tree* tree, void * value){
  assert(tree != NULL);

  struct rb_node * node = find(tree, tree->root, value);
  if(node == NULL){
    return false;
  }else{
    delete_node(tree, node);
    return true;
  }
}


/*
 * Dispose functions
 */

void rb_tree_dispose(struct rb_tree * tree){
  assert(tree != NULL);
  
  struct rb_node * node = tree->root;
  struct rb_node * next;
  while(node != NULL){
    if(node->left){
      next = node->left;
      node->left = NULL;
    }else if(node == node->right){
      next = node->right;
      node->right = NULL;
    }else{
      next = node->parent;
      free_node(tree, node);
    }
    node = next;
  }
}
