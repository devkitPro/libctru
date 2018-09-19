#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

void
rbtree_init(rbtree_t                 *tree,
            rbtree_node_comparator_t comparator)
{
  tree->root       = NULL;
  tree->comparator = comparator;
  tree->size       = 0;

  rbtree_validate(tree);
}
