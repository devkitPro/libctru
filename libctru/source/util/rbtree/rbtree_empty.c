#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

int
rbtree_empty(const rbtree_t *tree)
{
  rbtree_validate(tree);

  return tree->root == NULL;
}
