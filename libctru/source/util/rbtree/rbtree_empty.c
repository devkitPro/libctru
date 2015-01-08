#include <3ds/util/rbtree.h>

int
rbtree_empty(const rbtree_t *tree)
{
  return tree->root == NULL;
}
