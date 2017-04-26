#ifndef HTM_TREE_GEN_TREE_GEN_CONTEXT_H
#define HTM_TREE_GEN_TREE_GEN_CONTEXT_H

#include "node.hxx"
#include "arena.hxx"
#include "blk_writer.hxx"

/*  Tree generation context.
 */

struct tree_gen_context
{
#if FAST_ALLOC
  arena ar; /* node memory allocator */
#endif
  size_t nnodes;            /* number of nodes in the tree */
  uint64_t leafthresh;      /* maximum # of points per leaf */
  uint64_t poidx;           /* next post-order tree traversal index */
  uint64_t blockid[NLOD];   /* index of next block ID to assign for each LOD */
  blk_writer<disk_node> wr; /* node writer */

  tree_gen_context () = delete;
  tree_gen_context (const uint64_t Leafthresh, const std::string &file,
                    const size_t blksz)
      :
#if FAST_ALLOC
        ar (sizeof(mem_node)),
#endif
        nnodes (0), leafthresh (Leafthresh), poidx (0), wr (file, blksz)
  {
    for (int i = 0; i < NLOD; ++i)
      blockid[i] = 0;
  }
};

#endif
