/*  Called on a node when all points belonging to it have been accounted for.
 */

#include "../tree_gen_context.hxx"
#include "../node.hxx"
#include "layout_node.hxx"

void emit_node (mem_node *const node, tree_gen_context &ctx)
{
  int c;
  if (node->status > NODE_INIT)
    {
      return;
    }
  /* visit children */
  for (c = 0; c < 4; ++c)
    {
      if (node->child[c] != NULL)
        {
          emit_node (node->child[c], ctx);
        }
    }
  if (node->count < ctx.leafthresh)
    {
      /* if the node point count is too small,
         make it a leaf by deleting all children. */
      for (c = 0; c < 4; ++c)
        {
          if (node->child[c] != NULL)
            {
#if FAST_ALLOC
              ctx.ar.free (node->child[c]);
#else
              free (node->child[c]);
#endif
              node->child[c] = NULL;
            }
        }
      node->status = NODE_EMITTED;
    }
  else
    {
      /* otherwise, layout the subtree rooted at node */
      layout_node (node, ctx);
    }
}
