#include <sys/mman.h>

#include "htm.hxx"
#include "tinyhtm/tree.h"
#include "tinyhtm/varint.h"

#include "htm/_htm_s2ellipse_htmcov.hxx"
#include "_htm_subdivide.hxx"

extern "C" {

struct htm_range htm_tree_s2ellipse_range (const struct htm_tree *tree,
                                           const struct htm_s2ellipse *ellipse,
                                           enum htm_errcode *err)
{
  struct _htm_path path;
  struct htm_range range;

  range.min = 0;
  range.max = 0;
  if (tree == NULL || ellipse == NULL)
    {
      if (err != NULL)
        {
          *err = HTM_ENULLPTR;
        }
      range.max = -1;
      return range;
    }
  if (tree->index != MAP_FAILED)
    {
      range.max = htm_tree_s2ellipse_scan (tree, ellipse, err, NULL);
      if (range.max >= 0)
        {
          range.min = range.max;
        }
      return range;
    }

  for (int root = HTM_S0; root <= HTM_N3; ++root)
    {
      struct _htm_node *curnode = path.node;
      const unsigned char *s = tree->root[root];
      int level = 0;

      if (s == NULL)
        {
          /* root contains no points */
          continue;
        }
      _htm_path_root (&path, static_cast<htm_root>(root));

      while (1)
        {
          uint64_t curcount = htm_varint_decode (s);
          s += 1 + htm_varint_nfollow (*s);
          s += 1 + htm_varint_nfollow (*s);
          switch (_htm_s2ellipse_htmcov (curnode, ellipse))
            {
            case HTM_CONTAINS:
              if (level == 0)
                {
                  /* no need to consider other roots */
                  root = HTM_N3;
                }
              else
                {
                  /* no need to consider other children of parent */
                  curnode[-1].child = 4;
                }
            /* fall-through */
            case HTM_INTERSECT:
              if (level < 20 && curcount >= tree->leafthresh)
                {
                  s = _htm_subdivide (curnode, s);
                  if (s == NULL)
                    {
                      /* tree is invalid */
                      if (err != NULL)
                        {
                          *err = HTM_EINV;
                        }
                      range.max = -1;
                      return range;
                    }
                  ++level;
                  ++curnode;
                  continue;
                }
              range.max += (int64_t)curcount;
              break;
            case HTM_INSIDE:
              /* fully covered HTM triangle */
              range.min += (int64_t)curcount;
              range.max += (int64_t)curcount;
              break;
            default:
              /* HTM triangle does not intersect circle */
              break;
            }
        /* ascend towards the root */
        ascend:
          --level;
          --curnode;
          while (level >= 0 && curnode->child == 4)
            {
              --curnode;
              --level;
            }
          if (level < 0)
            {
              /* finished with this root */
              break;
            }
          s = _htm_subdivide (curnode, curnode->s);
          if (s == NULL)
            {
              /* no non-empty children remain */
              goto ascend;
            }
          ++level;
          ++curnode;
        }
    }
  if (err != NULL)
    {
      *err = HTM_OK;
    }
  return range;
}
}
