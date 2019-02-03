#include "rbtree_internal.h"

#include <3ds/svc.h>

#include <stdlib.h>

#define panic() do { \
  svcBreak(USERBREAK_PANIC); \
  abort(); \
} while(0)

void
rbtree_validate(const rbtree_t *tree)
{
  if(!tree)
    panic();

  // root node must be black
  if(!is_black(tree->root))
    panic();

  // root node's parent must be null
  if(tree->root)
  {
    if(get_parent(tree->root))
      panic();
  }

  // validate subtree starting at root node
  size_t size = 0;
  rbtree_node_validate(tree, tree->root, &size, NULL, NULL);

  // make sure we are tracking the correct number of nodes
  if(size != tree->size)
    panic();
}

void
rbtree_node_validate(const rbtree_t *tree, const rbtree_node_t *node, size_t *size, size_t *black, size_t *depth)
{
  if(!node) // implies is_black
  {
    if(black)
      *black = 1;

    if(depth)
      *depth = 1;

    return;
  }

  for(size_t i = 0; i < 32; ++i)
  {
    if(node->prefix[i])
      panic();

    if(node->suffix[i])
      panic();
  }

  rbtree_node_t *parent = get_parent(node);
  if(!parent)
  {
    // only the root node can have a null parent
    if(node != tree->root)
      panic();
  }
  else
  {
    // make sure this node is a child of its parent
    if(parent->child[LEFT] != node && parent->child[RIGHT] != node)
      panic();

    // if the parent is red, this node must be black
    if(is_red(parent) && !is_black(node))
      panic();
  }

  if(is_red(node))
  {
    // if this node is red, both children must be black
    if(!is_black(node->child[LEFT]))
      panic();

    if(!is_black(node->child[RIGHT]))
      panic();
  }

  if(node->child[LEFT])
  {
    // this node must be >= left child
    if((*(tree->comparator))(node, node->child[LEFT]) < 0)
      panic();
  }

  if(node->child[RIGHT])
  {
    // this node must be <= right child
    if((*(tree->comparator))(node, node->child[RIGHT]) > 0)
      panic();
  }

  // validate subtree at left child
  size_t left_size = 0;
  size_t left_black = 0;
  size_t left_depth = 0;
  rbtree_node_validate(tree, node->child[LEFT], &left_size, &left_black, &left_depth);

  // validate subtree at right child
  size_t right_size = 0;
  size_t right_black = 0;
  size_t right_depth = 0;
  rbtree_node_validate(tree, node->child[RIGHT], &right_size, &right_black, &right_depth);

  // size is left+right subtrees plus self
  if(size)
    *size = left_size + right_size + 1;

  // all possible paths to leaf nodes must have the same number of black nodes along the path
  if(left_black != right_black)
    panic();

  if(black)
    *black = left_black + (is_black(node) ? 1 : 0);

  // depth of one subtree must not exceed 2x depth of the other subtree
  if(left_depth < right_depth)
  {
    if(right_depth - left_depth > left_depth)
      panic();
  }
  else if(left_depth - right_depth > right_depth)
    panic();

  if(depth)
  {
    if(left_depth > right_depth)
      *depth = left_depth + 1;
    else
      *depth = right_depth + 1;
  }
}

void
rbtree_set_busy(rbtree_t *tree)
{
  LightLock_Lock(&tree->lock);
  if(tree->busy)
    panic();

  tree->busy = true;
  LightLock_Unlock(&tree->lock);
}

void
rbtree_clear_busy(rbtree_t *tree)
{
  LightLock_Lock(&tree->lock);
  if(!tree->busy)
    panic();

  tree->busy = false;
  LightLock_Unlock(&tree->lock);
}
