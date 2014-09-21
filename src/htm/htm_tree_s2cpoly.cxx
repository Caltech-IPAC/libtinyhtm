#include <sys/mman.h>

#include "tinyhtm/htm.h"
#include "tinyhtm/tree.h"
#include "tinyhtm/varint.h"
#include "htm.hxx"
#include "_htm_s2cpoly_htmcov.hxx"
#include "_htm_subdivide.hxx"
#include "htm/htm_s2cpoly_cv3_template.hxx"

template <typename T>
int64_t htm_tree_s2cpoly_template(const struct htm_tree *tree,
                                  const struct htm_s2cpoly *poly,
                                  enum htm_errcode *err,
                                  htm_callback callback)
{

    double stackab[2*256 + 4];
    struct _htm_path path;
    double *ab;
    size_t nb;
    int64_t count;

    if (tree == NULL || poly == NULL) {
        if (err != NULL) {
            *err = HTM_ENULLPTR;
        }
        return -1;
    }
    if (tree->index == MAP_FAILED) {
      return htm_tree_s2cpoly_scan(tree, poly, err, callback);
    }
    nb = (2 * poly->n + 4) * sizeof(double);
    if (nb > sizeof(stackab)) {
        ab = (double *) malloc(nb);
        if (ab == NULL) {
            if (err != NULL) {
                *err = HTM_ENOMEM;
            }
            return -1;
        }
    } else {
        ab = stackab;
    }
    count = 0;

    for (int root = HTM_S0; root <= HTM_N3; ++root) {
        struct _htm_node *curnode = path.node;
        const unsigned char *s = tree->root[root];
        uint64_t index = 0;
        int level = 0;

        if (s == NULL) {
            /* root contains no points */
            continue;
        }
        _htm_path_root(&path, static_cast<htm_root>(root));

        while (1) {
            uint64_t curcount = htm_varint_decode(s);
            s += 1 + htm_varint_nfollow(*s);
            index += htm_varint_decode(s);
            s += 1 + htm_varint_nfollow(*s);
            curnode->index = index;

            enum _htm_cov coverage=_htm_s2cpoly_htmcov(curnode, poly, ab);
            if(coverage==HTM_CONTAINS)
              {
                if (level == 0) {
                  /* no need to consider other roots */
                  root = HTM_N3;
                } else {
                  /* no need to consider other children of parent */
                  curnode[-1].child = 4;
                }
              }
            if(coverage==HTM_CONTAINS || coverage==HTM_INTERSECT)
              {
                if (level < 20 && curcount >= tree->leafthresh) {
                  s = _htm_subdivide(curnode, s);
                  if (s == NULL) {
                    /* tree is invalid */
                    if (ab != stackab) {
                      free(ab);
                    }
                    if (err != NULL) {
                      *err = HTM_EINV;
                    }
                    return -1;
                  }
                  ++level;
                  ++curnode;
                  continue;
                }
              }
            if(!callback && coverage==HTM_INSIDE)
              {
                /* fully covered HTM triangle */
                count += (int64_t) curcount;
              }
            else if(coverage==HTM_CONTAINS || coverage==HTM_INTERSECT
                    || coverage==HTM_INSIDE)
              {
                uint64_t i;
                for (i = index; i < index + curcount; ++i) {
                  if (htm_s2cpoly_cv3_template<T>(poly, reinterpret_cast<T*>
                                      (static_cast<char*>(tree->entries)
                                       + i*tree->entry_size))) {
                    if(!callback
                       || callback(static_cast<char*>(tree->entries)
                                   + i*tree->entry_size,
                                   tree->num_elements_per_entry,
                                   tree->element_types, tree->element_names))
                      ++count;
                  }
                }
              }
            
            /* ascend towards the root */
ascend:
            --level;
            --curnode;
            while (level >= 0 && curnode->child == 4) {
                --curnode;
                --level;
            }
            if (level < 0) {
                /* finished with this root */
                break;
            }
            index = curnode->index;
            s = _htm_subdivide(curnode, curnode->s);
            if (s == NULL) {
                /* no non-empty children remain */
                goto ascend;
            }
            ++level;
            ++curnode;
        }
    }
    if (ab != stackab) {
        free(ab);
    }
    if (err != NULL) {
        *err = HTM_OK;
    }
    return count;
}

extern "C" {
int64_t htm_tree_s2cpoly(const struct htm_tree *tree,
                         const struct htm_s2cpoly *poly,
                         enum htm_errcode *err,
                         htm_callback callback)
{
  if(tree->element_types.at(0)==H5::PredType::NATIVE_DOUBLE)
    {
      htm_tree_s2cpoly_template<double>(tree,poly,err,callback);
    }
  else if(tree->element_types.at(0)==H5::PredType::NATIVE_FLOAT)
    {
      htm_tree_s2cpoly_template<float>(tree,poly,err,callback);
    }
  else
    {
      if(err!=nullptr)
        {
          *err=HTM_ETREE;
        }
      return -1;
    }
  return 0;
}

}

