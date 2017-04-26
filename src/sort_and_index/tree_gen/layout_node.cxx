#include <algorithm>
#include "../node.hxx"
#include "../tree_gen_context.hxx"
#include "child_info.hxx"
#include "assign_block.hxx"

uint32_t estimate_node_size (const struct mem_node *const node,
                             const uint32_t nchild);

void layout_node (mem_node *const node, tree_gen_context &ctx)
{
  struct child_info cinfo[4];
  int c, nchild;

  if (node->status > NODE_EMITTED)
    {
      return;
    }

  /* visit children */
  for (c = 0, nchild = 0; c < 4; ++c)
    {
      if (node->child[c] != NULL)
        {
          layout_node (node->child[c], ctx);
          cinfo[nchild].node = node->child[c];
          cinfo[nchild].idx = c;
          ++nchild;
        }
    }

  /* update status and assign post-order index. */
  node->status = NODE_LAID_OUT;
  node->id.block[5] = ++ctx.poidx;

  /* Clark & Munro at every level-of-detail */
  if (nchild == 0)
    {
      /* leaf */
      int lod;
      const uint32_t nodesz = estimate_node_size (node, nchild);
      const uint32_t info = make_block_info (nodesz, 1u);
      for (lod = 0; lod < NLOD; ++lod)
        {
          node->blockinfo[lod] = info;
          if (nodesz > layout_size[lod])
            {
              uint64_t blockid = ++ctx.blockid[lod];
              assign_block (ctx, node, blockid, lod);
            }
        }
    }
  else
    {
      /* internal node */
      int lod;
      const uint32_t nodesz = estimate_node_size (node, nchild);
      for (lod = 0; lod < NLOD; ++lod)
        {
          uint64_t blockid;
          uint32_t totsz = nodesz;
          int close = 0, endclose = nchild;

          for (c = 0; c < nchild; ++c)
            {
              uint32_t s;
              struct mem_node *tmp = cinfo[c].node;
              s = get_block_size (tmp->blockinfo[lod]);
              cinfo[c].size = s;
              cinfo[c].depth = get_block_depth (tmp->blockinfo[lod]);
              totsz += s;
            }
          std::sort (cinfo, cinfo + nchild);

          if (cinfo[0].depth == cinfo[nchild - 1].depth)
            {
              /* all children have the same block depth */
              if (totsz <= layout_size[lod])
                {
                  /* children and parent all fit in one block; merge them */
                  node->blockinfo[lod]
                      = make_block_info (totsz, cinfo[0].depth);
                  continue;
                }
              else
                {
                  /* cannot fit family into one block: scan children
                     from smallest to largest, placing as many as possible
                     in the same block as the parent. */
                  totsz = nodesz;
                  for (close = 0; close < nchild - 1; ++close)
                    {
                      if (totsz + cinfo[close].size > layout_size[lod])
                        {
                          break;
                        }
                      totsz += cinfo[close].size;
                    }
                  /* increase block depth of parent by 1 */
                  node->blockinfo[lod]
                      = make_block_info (totsz, cinfo[0].depth + 1);
                }
            }
          else
            {
              /* nchild > 1, not all children have the same block depth */
              totsz = nodesz;
              for (endclose = nchild - 1; endclose > 0; --endclose)
                {
                  totsz += cinfo[endclose].size;
                  if (cinfo[endclose - 1].depth != cinfo[nchild - 1].depth)
                    {
                      break;
                    }
                }
              if (totsz < layout_size[lod])
                {
                  /* merge parent and largest-depth children */
                  node->blockinfo[lod]
                      = make_block_info (totsz, cinfo[nchild - 1].depth);
                }
              else
                {
                  /* fresh block for parent, increase block depth by 1 */
                  node->blockinfo[lod]
                      = make_block_info (nodesz, cinfo[nchild - 1].depth + 1);
                  endclose = nchild;
                }
            }
          /* scan remaining children from smallest to largest, merging
             runs of children into a single block where possible. */
          totsz = cinfo[close].size;
          for (c = close + 1; c < endclose; ++c)
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
              else
                {
                  totsz += cinfo[c].size;
                }
            }
          blockid = ++ctx.blockid[lod];
          for (; close < endclose; ++close)
            {
              assign_block (ctx, cinfo[close].node, blockid, lod);
            }
        }
    }
}
