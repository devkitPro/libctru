#include <3ds/util/rbtree.h>
#include "rbtree_internal.h"

static rbtree_node_t*
do_insert(rbtree_t      *tree,
          rbtree_node_t *node,
          int           multi)
{
  rbtree_set_busy(tree);
  rbtree_validate(tree);

  node->nodeTag[3]   = 'N';
  node->nodeTag[2]   = 'O';
  node->nodeTag[1]   = 'D';
  node->nodeTag[0]   = 'E';
  node->parent_color = 0;
  node->child[LEFT]  = NULL;
  node->child[RIGHT] = NULL;
  node->tree         = tree;

  rbtree_node_t *original = node;
  rbtree_node_t **tmp     = &tree->root;
  rbtree_node_t *parent   = NULL;
  rbtree_node_t *save     = NULL;

  while(*tmp)
  {
    int cmp = (*(tree->comparator))(node, *tmp);
    parent  = *tmp;

    if(cmp < 0)
      tmp = &((*tmp)->child[LEFT]);
    else if(cmp > 0)
      tmp = &((*tmp)->child[RIGHT]);
    else
    {
      if(!multi)
        save = *tmp;

      tmp = &((*tmp)->child[LEFT]);
    }
  }

  if(save)
  {
    node->nodeTag[3]   = 'D';
    node->nodeTag[2]   = 'O';
    node->nodeTag[1]   = 'N';
    node->nodeTag[0]   = 'E';
    node->parent_color = 0;
    node->child[LEFT]  = NULL;
    node->child[RIGHT] = NULL;
    node->tree         = NULL;

    rbtree_validate(tree);
    rbtree_clear_busy(tree);

    return save;
  }

  *tmp = node;

  node->child[LEFT] = node->child[RIGHT] = NULL;
  set_parent(node, parent);

  set_red(node);

  while(is_red((parent = get_parent(node))))
  {
    rbtree_node_t *grandparent = get_parent(parent);
    int           left = (parent == grandparent->child[LEFT]);
    rbtree_node_t *uncle = grandparent->child[left];

    if(is_red(uncle))
    {
      set_black(uncle);
      set_black(parent);
      set_red(grandparent);

      node = grandparent;
    }
    else
    {
      if(parent->child[left] == node)
      {
        rbtree_node_t *tmp;

        rbtree_rotate(tree, parent, left);

        tmp    = parent;
        parent = node;
        node   = tmp;
      }

      set_black(parent);
      set_red(grandparent);
      rbtree_rotate(tree, grandparent, !left);
    }
  }

  set_black(tree->root);

  tree->size += 1;

  rbtree_validate(tree);
  rbtree_clear_busy(tree);

  return original;
}

rbtree_node_t*
rbtree_insert(rbtree_t      *tree,
              rbtree_node_t *node)
{
  return do_insert(tree, node, 0);
}

void
rbtree_insert_multi(rbtree_t      *tree,
                    rbtree_node_t *node)
{
  do_insert(tree, node, 1);
}
