#pragma once

#include <stdint.h>
#include <stddef.h>

#define rbtree_item(ptr, type, member) \
  ((type*)(((char*)ptr) - offsetof(type, member)))

typedef struct rbtree      rbtree_t;
typedef struct rbtree_node rbtree_node_t;

typedef void (*rbtree_node_destructor_t)(rbtree_node_t *Node);
typedef int  (*rbtree_node_comparator_t)(const rbtree_node_t *lhs,
                                         const rbtree_node_t *rhs);
struct rbtree_node
{
  uintptr_t      parent_color;
  rbtree_node_t  *child[2];
};

struct rbtree
{
  rbtree_node_t            *root;
  rbtree_node_comparator_t comparator;
  size_t                   size;
};

#ifdef __cplusplus
extern "C" {
#endif

void
rbtree_init(rbtree_t                 *tree,
            rbtree_node_comparator_t comparator);

int
rbtree_empty(const rbtree_t *tree);

size_t
rbtree_size(const rbtree_t *tree);

__attribute__((warn_unused_result))
rbtree_node_t*
rbtree_insert(rbtree_t      *tree,
              rbtree_node_t *node);

void
rbtree_insert_multi(rbtree_t      *tree,
                    rbtree_node_t *node);

rbtree_node_t*
rbtree_find(const rbtree_t      *tree,
            const rbtree_node_t *node);

rbtree_node_t*
rbtree_min(const rbtree_t *tree);

rbtree_node_t*
rbtree_max(const rbtree_t *tree);

rbtree_node_t*
rbtree_node_next(const rbtree_node_t *node);

rbtree_node_t*
rbtree_node_prev(const rbtree_node_t *node);

rbtree_node_t*
rbtree_remove(rbtree_t                 *tree,
              rbtree_node_t            *node,
              rbtree_node_destructor_t destructor);

void
rbtree_clear(rbtree_t                 *tree,
             rbtree_node_destructor_t destructor);

#ifdef __cplusplus
}
#endif
