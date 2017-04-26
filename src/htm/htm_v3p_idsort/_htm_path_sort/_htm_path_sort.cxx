/*  Perform a depth-first traversal of an HTM tree. At each step of the
    traversal, the input position list is partitioned into the list of
    points inside the current HTM node and those outside - the full
    depth-first traverals of the HTM tree therefore yields a list of
    positions sorted on their HTM indexes. This can be faster than computing
    HTM indexes one at a time when inputs are clustered spatially, since the
    boundary representation of a given HTM triangle is computed at most once.
 */

#include "htm.hxx"

struct htm_v3p *_htm_partition (const struct htm_v3 *plane,
                                struct htm_v3p *beg, struct htm_v3p *end);

void _htm_path_sort (struct _htm_path *path, struct htm_v3p *begin,
                     struct htm_v3p *end, int64_t *ids, int level)
{
  struct _htm_node *const root = path->node;
  struct _htm_node *const leaf = root + level;
  struct _htm_node *curnode = path->node;
  struct htm_v3p *beg = begin;

  curnode->end = end;

  while (1)
    {
      if (curnode != leaf)
        {
          /* Not a leaf node, so continue descending the tree.
             Mid-points and edge normals are computed on-demand. */
          int child = curnode->child;
          if (child == 0)
            {
              _htm_node_prep0 (curnode);
              end = _htm_partition (&curnode->mid_edge[1], beg, end);
              if (beg < end)
                {
                  _htm_node_make0 (curnode);
                  ++curnode;
                  curnode->end = end;
                  continue;
                }
              end = curnode->end;
            }
          if (child <= 1)
            {
              _htm_node_prep1 (curnode);
              end = _htm_partition (&curnode->mid_edge[2], beg, end);
              if (beg < end)
                {
                  _htm_node_make1 (curnode);
                  ++curnode;
                  curnode->end = end;
                  continue;
                }
              end = curnode->end;
            }
          if (child <= 2)
            {
              _htm_node_prep2 (curnode);
              end = _htm_partition (&curnode->mid_edge[0], beg, end);
              if (beg < end)
                {
                  _htm_node_make2 (curnode);
                  ++curnode;
                  curnode->end = end;
                  continue;
                }
              end = curnode->end;
            }
          if (beg < end)
            {
              _htm_node_make3 (curnode);
              ++curnode;
              curnode->end = end;
              continue;
            }
        }
      else
        {
          /* reached a leaf triangle - all points between beg and end are
             inside and have the same HTM index. */
          int64_t id = curnode->id;
          size_t i = beg - begin;
          size_t j = end - begin;
          for (; i < j; ++i)
            {
              ids[i] = id;
            }
          beg = end;
        }
      /* walk back up the path until we find a node which
         still contains unsorted points. */
      do
        {
          if (curnode == root)
            {
              return;
            }
          --curnode;
          end = curnode->end;
        }
      while (beg == end);
    }
}
