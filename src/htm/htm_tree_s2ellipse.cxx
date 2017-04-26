#include <sys/mman.h>

#include "tinyhtm/tree.h"
#include "tinyhtm/varint.h"

#include "_htm_s2ellipse_htmcov.hxx"
#include "_htm_subdivide.hxx"
#include "htm/htm_s2ellipse_cv3_template.hxx"

template <typename T>
int64_t htm_tree_s2ellipse_template (const struct htm_tree *tree,
                                     const struct htm_s2ellipse *ellipse,
                                     enum htm_errcode *err,
                                     htm_callback callback)
{
  struct _htm_path path;
  int64_t count;

  if (tree == NULL || ellipse == NULL)
    {
      if (err != NULL)
        {
          *err = HTM_ENULLPTR;
        }
      return -1;
    }
  if (tree->index == MAP_FAILED)
    {
      return htm_tree_s2ellipse_scan (tree, ellipse, err, callback);
    }

  count = 0;

  for (int root = HTM_S0; root <= HTM_N3; ++root)
    {
      struct _htm_node *curnode = path.node;
      const unsigned char *s = tree->root[root];
      uint64_t index = 0;
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
          index += htm_varint_decode (s);
          s += 1 + htm_varint_nfollow (*s);
          curnode->index = index;

          enum _htm_cov coverage = _htm_s2ellipse_htmcov (curnode, ellipse);
          if (coverage == HTM_CONTAINS)
            {
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
            }
          if (coverage == HTM_CONTAINS || coverage == HTM_INTERSECT)
            {
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
                      return -1;
                    }
                  ++level;
                  ++curnode;
                  continue;
                }
            }
          if (!callback && coverage == HTM_INSIDE)
            {
              /* fully covered HTM triangle */
              count += (int64_t)curcount;
            }
          else if (coverage == HTM_CONTAINS || coverage == HTM_INTERSECT
                   || coverage == HTM_INSIDE)
            {
              uint64_t i;
              for (i = index; i < index + curcount; ++i)
                {
                  if (coverage == HTM_INSIDE
                      || htm_s2ellipse_cv3_template (
                             ellipse, reinterpret_cast<T *>(
                                          static_cast<char *>(tree->entries)
                                          + i * tree->entry_size)))
                    {
                      if (!callback
                          || callback (static_cast<char *>(tree->entries)
                                       + i * tree->entry_size))
                        ++count;
                    }
                }
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
          index = curnode->index;
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
  return count;
}

extern "C" {
int64_t htm_tree_s2ellipse (const struct htm_tree *tree,
                            const struct htm_s2ellipse *ellipse,
                            enum htm_errcode *err, htm_callback callback)
{
  if (tree->element_types.at (0) == H5::PredType::NATIVE_DOUBLE)
    {
      htm_tree_s2ellipse_template<double>(tree, ellipse, err, callback);
    }
  else if (tree->element_types.at (0) == H5::PredType::NATIVE_FLOAT)
    {
      htm_tree_s2ellipse_template<float>(tree, ellipse, err, callback);
    }
  else
    {
      if (err != nullptr)
        {
          *err = HTM_ETREE;
        }
      return -1;
    }
  return 0;
}
}
