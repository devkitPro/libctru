#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

void
rbtree_init(rbtree_t                 *tree,
            rbtree_node_comparator_t comparator)
{
  tree->treeTag[3] = 'T';
  tree->treeTag[2] = 'R';
  tree->treeTag[1] = 'E';
  tree->treeTag[0] = 'E';
  tree->root       = NULL;
  tree->comparator = comparator;
  tree->size       = 0;
  tree->busy       = false;
  LightLock_Init(&tree->lock);

  rbtree_validate(tree);
}
