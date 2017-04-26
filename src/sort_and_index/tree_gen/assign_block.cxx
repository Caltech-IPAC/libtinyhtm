/*  Assigns a block ID at the specified level-of-detail to all nodes in
    the sub-tree rooted at n that do not already have a block ID at that
    level-of-detail. When assigning a block ID to a node, check whether all
    block IDs are now valid and if so write the node out to disk.
    Children of nodes that are written are destroyed.
 */

#include <cstring>
#include "../tree_gen_context.hxx"
#include "../node.hxx"

void assign_block (tree_gen_context &ctx, mem_node *const n,
                   const uint64_t blockid, const int lod)
{
  int i;
  if (n->id.block[lod] != 0)
    {
      return;
    }
  /* visit children */
  for (i = 0; i < 4; ++i)
    {
      if (n->child[i] != NULL)
        {
          assign_block (ctx, n->child[i], blockid, lod);
        }
    }
  n->id.block[lod] = blockid;
  for (i = 0; i < NLOD; ++i)
    {
      if (n->id.block[i] == 0)
        {
          return;
        }
    }
  /* write node to disk */
  {
    struct disk_node d;
    d.id = n->id;
    d.count = n->count;
    d.index = n->index;
    for (i = 0; i < 4; ++i)
      {
        struct mem_node *tmp = n->child[i];
        if (tmp != NULL)
          {
            /* copy id of child to disk node, throw away child */
            d.child[i] = tmp->id;
#if FAST_ALLOC
            ctx.ar.free (tmp);
#else
            free (tmp);
#endif
            n->child[i] = NULL;
          }
        else
          {
            memset (&d.child[i], 0, sizeof(struct node_id));
          }
      }
    ctx.wr.append (&d);
    ++ctx.nnodes;
  }
}
