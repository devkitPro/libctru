#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

rbtree_node_t*
rbtree_find(const rbtree_t      *tree,
            const rbtree_node_t *node)
{
  rbtree_set_busy((rbtree_t*)tree);
  rbtree_validate(tree);

  rbtree_node_t *tmp  = tree->root;
  rbtree_node_t *save = NULL;

  while(tmp)
  {
    int rc = (*tree->comparator)(node, tmp);
    if(rc < 0)
    {
      tmp = tmp->child[LEFT];
    }
    else if(rc > 0)
    {
      tmp = tmp->child[RIGHT];
    }
    else
    {
      save = tmp;
      tmp = tmp->child[LEFT];
    }
  }

  rbtree_validate(tree);
  rbtree_clear_busy((rbtree_t*)tree);

  return save;
}
