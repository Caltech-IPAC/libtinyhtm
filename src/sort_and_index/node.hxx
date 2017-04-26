#ifndef HTM_TREE_GEN_NODE_H
#define HTM_TREE_GEN_NODE_H

#include <cassert>
#include <cstdint>
#include "tinyhtm.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  Number of levels-of-detail used by Split-and-Refine.
 */
#define NLOD 5

/*  Tree layout block sizes in bytes, from largest to smallest.
 */
static const uint32_t layout_size[NLOD] = {
  2097152, /* 2MiB: large page size for modern x86 processors. */
  65536,   /* Between small and large page size. Chosen in hopes
              of improving OS disk prefetch effectiveness. */
  4096,    /* 4KiB: default page size for modern x86 processors */
  256,     /* Between cache line and small page sizes. Chosen in hopes
              of improving HW cache-line prefetch effectiveness. */
  64       /* L1/L2/L3 cache-line size for modern x86 processors. */
};

/*  A node ID, consisting of NLOD block IDs (one per layout block size)
    and the index of the node in a post-order traversal. This last index
    makes node IDs unique (more than one node might fit in a block at
    the finest LOD).
 */
struct node_id
{
  uint64_t block[NLOD + 1];

  /*  Returns 1 if the first node ID is less than the second. We say that
      a node N1 is less than N2 if the string of block IDs for N1 is
      lexicographically less than the string for N2.
   */
  bool operator<(const node_id &n) const
  {
    for (int i = 0; i < NLOD + 1; ++i)
      {
        if (block[i] < n.block[i])
          {
            return true;
          }
        else if (block[i] > n.block[i])
          {
            break;
          }
      }
    return false;
  }
} HTM_ALIGNED (16);

/*  On-disk representation of a tree node.
 */
struct disk_node
{
  struct node_id id;
  uint64_t count;
  uint64_t index;
  struct node_id child[4];
  bool operator<(const disk_node &d) const { return id < d.id; }
} HTM_ALIGNED (16);

/*  Returns 1 if the given node ID corresponds to an empty child
    (all block IDs are zero).
 */
HTM_INLINE int node_empty (const struct node_id *const id)
{
  int i;
  for (i = 0; i < NLOD + 1; ++i)
    {
      if (id->block[i] != 0)
        {
          return 0;
        }
    }
  return 1;
}

/*  Returns 1 if the given node IDs are equal.
 */
HTM_INLINE int node_id_eq (const struct node_id *const id1,
                           const struct node_id *const id2)
{
  int i;
  for (i = 0; i < NLOD + 1; ++i)
    {
      if (id1->block[i] != id2->block[i])
        {
          return 0;
        }
    }
  return 1;
}

void disk_node_sort (void *data, size_t nbytes);
int disk_node_cmp (const void *n1, const void *n2);

/* ---- In-memory node representation ---- */

/*  Status of an in-memory tree node.
 */
enum node_status
{
  NODE_INIT = 0, /* node is minty fresh */
  NODE_EMITTED,  /* node has been processed by emit_node() */
  NODE_LAID_OUT, /* node has been processed by layout_node() */
};

/*  In-memory representation of a tree node.  Note that block size/depth
    is packed into a single 32 bit integer; as a result, nodes occupy
    exactly 256 bytes and are nicely cache aligned.
 */
struct mem_node
{
  struct node_id id; /* Hierarchical ID for node */
  int64_t htmid;     /* HTM ID of node */
  uint64_t index;    /* File offset of first point in node */
  uint64_t count;    /* Number of points in node */
  enum node_status status;
  uint32_t blockinfo[NLOD]; /* Clark & Munro: block depth (8 MSBs) and
                               block size (24 LSBs) for each LOD. */
  struct mem_node *child[4];
} HTM_ALIGNED (16);

/* get/set block size/depth from 32 bit blockinfo */
HTM_INLINE uint32_t get_block_size (uint32_t blockinfo)
{
  return blockinfo & 0xffffff;
}
HTM_INLINE uint8_t get_block_depth (uint32_t blockinfo)
{
  return blockinfo >> 24;
}
HTM_INLINE uint32_t make_block_info (uint32_t size, uint8_t depth)
{
  return (size & 0xffffff) | (((uint32_t)depth) << 24);
}

#ifdef __cplusplus
}
#endif

#endif
