/*  Assigns block IDs to the HTM root nodes.
 */

#include <algorithm>
#include "../tree_gen_context.hxx"
#include "../tree_root.hxx"
#include "child_info.hxx"
#include "assign_block.hxx"

void finish_root (tree_root &super, tree_gen_context &ctx)
{
  struct child_info cinfo[8];
  int c, close, nchild, lod;

  for (c = 0, nchild = 0; c < 8; ++c)
    {
      struct mem_node *tmp = super.child[c];
      if (tmp != NULL)
        {
          super.count += tmp->count;
          cinfo[nchild].node = tmp;
          cinfo[nchild].idx = c;
          ++nchild;
        }
    }
  /* Clark & Munro for each level of detail */
  for (lod = 0; lod < NLOD; ++lod)
    {
      uint64_t blockid;
      uint32_t totsz;
      for (c = 0; c < nchild; ++c)
        {
          struct mem_node *tmp = cinfo[c].node;
          cinfo[c].size = get_block_size (tmp->blockinfo[lod]);
          cinfo[c].depth = get_block_depth (tmp->blockinfo[lod]);
        }
      std::sort (cinfo, cinfo + nchild);
      /* scan children from smallest to largest, merging
         as many as possible into blocks. */
      close = 0;
      totsz = cinfo[0].size;
      for (c = 1; c < nchild; ++c)
        {
          if (totsz + cinfo[c].size > layout_size[lod])
            {
              blockid = ++ctx.blockid[lod];
              for (; close < c; ++close)
                {
                  assign_block (ctx, cinfo[close].node, blockid, lod);
                }
              totsz = cinfo[c].size;
            }
        }
      blockid = ++ctx.blockid[lod];
      for (; close < nchild; ++close)
        {
          assign_block (ctx, cinfo[close].node, blockid, lod);
        }
    }
  /* At this point, all nodes are guaranteed to have been written to disk.
     Copy HTM root node IDs to the super root and then throw them away. */
  for (c = 0; c < nchild; ++c)
    {
      super.childid[cinfo[c].idx] = cinfo[c].node->id;
#if FAST_ALLOC
      ctx.ar.free (cinfo[c].node);
#else
      free (cinfo[c].node);
#endif
      super.child[cinfo[c].idx] = NULL;
    }
}
