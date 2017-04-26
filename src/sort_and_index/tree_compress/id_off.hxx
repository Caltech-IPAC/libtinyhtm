/*  Mapping from a node ID to a relative file offset.
 */

#ifndef HTM_TREE_GEN_ID_OFF_H
#define HTM_TREE_GEN_ID_OFF_H

#include "../node.hxx"

struct id_off
{
  struct node_id id;
  uint64_t off;
  struct id_off *next;
} HTM_ALIGNED (16);

#endif
