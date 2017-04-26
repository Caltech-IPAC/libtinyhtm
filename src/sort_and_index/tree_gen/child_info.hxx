#ifndef HTM_TREE_GEN_CHILD_INFO_H
#define HTM_TREE_GEN_CHILD_INFO_H

#include <cstdint>

struct child_info
{
  struct mem_node *node;
  uint32_t size;
  uint8_t depth;
  int8_t idx;
  bool operator<(const child_info &c) const
  {
    return depth < c.depth || (depth == c.depth && size < c.size);
  }
};

#endif
