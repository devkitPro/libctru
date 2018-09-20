#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

static inline rbtree_node_t*
do_minmax(const rbtree_t *tree,
          int            max)
{
  rbtree_set_busy((rbtree_t*)tree);
  rbtree_validate(tree);

  rbtree_node_t *node = tree->root;

  if(!node)
  {
    rbtree_clear_busy((rbtree_t*)tree);
    return NULL;
  }

  while(node->child[max])
    node = node->child[max];

  rbtree_clear_busy((rbtree_t*)tree);
  return node;
}

rbtree_node_t*
rbtree_min(const rbtree_t *tree)
{
  return do_minmax(tree, LEFT);
}

rbtree_node_t*
rbtree_max(const rbtree_t *tree)
{
  return do_minmax(tree, RIGHT);
}
