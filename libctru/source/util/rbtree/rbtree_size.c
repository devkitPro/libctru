#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

size_t
rbtree_size(const rbtree_t *tree)
{
  rbtree_set_busy((rbtree_t*)tree);
  rbtree_validate(tree);

  size_t size = tree->size;

  rbtree_clear_busy((rbtree_t*)tree);
  return size;
}
