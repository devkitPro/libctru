extern "C"
{
#include <3ds/svc.h>
#include <3ds/util/rbtree.h>
#include "../rbtree_internal.h"
}

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <limits>
#include <random>
#include <vector>

namespace
{
struct IntNode
{
  IntNode(int value) : value(value)
  {}

  const int value;
  rbtree_node_t node;
};

IntNode *getIntNode(rbtree_node_t *node)
{
  return rbtree_item(node, IntNode, node);
}

const IntNode *getIntNode(const rbtree_node_t *node)
{
  return rbtree_item(node, IntNode, node);
}

void IntNodeDestructor(rbtree_node_t *node)
{
  delete getIntNode(node);
}

int IntNodeComparator(const rbtree_node_t *lhs, const rbtree_node_t *rhs)
{
  auto left = getIntNode(lhs)->value;
  auto right = getIntNode(rhs)->value;

  if(left < right)
    return -1;
  if(right < left)
    return 1;
  return 0;
}

#if 0
void printNode(const rbtree_node_t *node, int indent)
{
  if(!node)
  {
    std::printf("%*s(nil) black\n", indent, "");
    return;
  }

  std::printf("%*s%d %s\n", indent, "", getIntNode(node)->value, is_black(node) ? "black" : "red");

  printNode(node->child[LEFT], indent + 2);
  printNode(node->child[RIGHT], indent + 2);
}

void printTree(const rbtree_t *tree)
{
  std::printf("==========\n");
  printNode(tree->root, 0);
}
#endif
}

void svcBreak(UserBreakType breakReason)
{
  std::abort();
}

int main(int argc, char *argv[])
{
  std::default_random_engine eng;

  {
    std::random_device rand;
    std::random_device::result_type seedBuffer[32];
    for(auto &d : seedBuffer)
      d = rand();

    std::seed_seq seed(std::begin(seedBuffer), std::end(seedBuffer));
    eng.seed(seed);
  }

  auto dist = std::bind(std::uniform_int_distribution<int>(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()), std::ref(eng));
  auto remove = std::bind(std::uniform_int_distribution<std::size_t>(0, 100), std::ref(eng));

  rbtree_t tree;
  rbtree_init(&tree, IntNodeComparator);

  for(std::size_t chance = 0; chance <= 100; chance += 25)
  {
    std::printf("Chance %zu\n", chance);

    std::vector<IntNode*> nodes;
    for(std::size_t i = 0; i < 10000; ++i)
    {
      auto node = new IntNode(dist());
      if(rbtree_insert(&tree, &node->node) != &node->node)
        delete node;
      else
        nodes.emplace_back(node);

      // remove random node
      if(remove() <= chance)
      {
        auto it = std::begin(nodes) + (eng() % nodes.size());
        rbtree_remove(&tree, &(*it)->node, IntNodeDestructor);
        nodes.erase(it);
      }

//      printTree(&tree);
    }
  }

  std::printf("Ended with %zu nodes in tree\n", rbtree_size(&tree));

  rbtree_clear(&tree, IntNodeDestructor);
}
