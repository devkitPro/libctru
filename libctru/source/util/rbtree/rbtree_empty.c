#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

int
rbtree_empty(const rbtree_t *tree)
{
  rbtree_set_busy((rbtree_t*)tree);
  rbtree_validate(tree);

  bool empty = !tree->root;

  rbtree_clear_busy((rbtree_t*)tree);
  return empty;
}
