#include <3ds/util/rbtree.h>

size_t
rbtree_size(const rbtree_t *tree)
{
  return tree->size;
}
