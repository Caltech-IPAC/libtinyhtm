#ifndef HTM_TREE_GEN_TREE_ROOT_H
#define HTM_TREE_GEN_TREE_ROOT_H

#include "node.hxx"

/*  Container for the 8 level-0 HTM tree nodes.
 */
struct tree_root
{
  uint64_t count; /* Total number of points in tree */
  struct mem_node *child[8];
  struct node_id childid[8];
};

#endif
