/*  Adds a new node to root (one of S0,S1,S2,S3,N0,N1,N2,N3).
 */

#include <stdexcept>
#include <cassert>
#include <cstring>
#include "emit_node.hxx"

void add_node (mem_node *const root, tree_gen_context &ctx, int64_t htmid,
               int64_t count, int64_t index)
{
  struct mem_node *node;
  int lvl = 0;

  for (lvl = 0, node = root; lvl < 20; ++lvl)
    {
      /* keep subdividing */
      int i, c;
      node->count += count;
      c = (htmid >> 2 * (19 - lvl)) & 3;
      for (i = 0; i < c; ++i)
        {
          struct mem_node *tmp = node->child[i];
          if (tmp != NULL && tmp->status == NODE_INIT)
            {
              emit_node (node->child[i], ctx);
            }
        }
      index -= node->index; /* relativize index */
      if (node->child[c] != NULL)
        {
          node = node->child[c];
        }
      else
        {
/* create child node */
#if FAST_ALLOC
          struct mem_node *child = (struct mem_node *)ctx.ar.alloc ();
#else
          struct mem_node *child
              = (struct mem_node *)malloc (sizeof(struct mem_node));
          if (child == NULL)
            {
              throw std::runtime_error ("malloc() failed");
            }
#endif
          memset (child, 0, sizeof(struct mem_node));
          child->index = index;
          child->htmid = node->htmid * 4 + c;
          node->child[c] = child;
          node = child;
        }
    }
  assert (node->htmid == htmid);
  node->count = count;
}
