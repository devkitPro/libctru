#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

size_t
rbtree_size(const rbtree_t *tree)
{
  rbtree_validate(tree);

  return tree->size;
}
