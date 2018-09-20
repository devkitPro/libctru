#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

void
rbtree_clear(rbtree_t                 *tree,
             rbtree_node_destructor_t destructor)
{
  rbtree_set_busy(tree);
  rbtree_validate(tree);

  rbtree_node_t *node = tree->root;

  while(tree->root)
  {
    while(node->child[LEFT])
      node = node->child[LEFT];

    if(node->child[RIGHT])
      node = node->child[RIGHT];
    else
    {
      rbtree_node_t *parent = get_parent(node);

      if(!parent)
        tree->root = NULL;
      else
        parent->child[node != parent->child[LEFT]] = NULL;

      if(destructor)
        (*destructor)(node);

      node = parent;
    }
  }

  tree->size = 0;

  rbtree_validate(tree);
  rbtree_clear_busy(tree);
}
