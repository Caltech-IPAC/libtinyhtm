#include <sys/mman.h>

#include "tinyhtm/htm.h"
#include "tinyhtm/tree.h"
#include "tinyhtm/varint.h"
#include "htm.hxx"

#include "htm/_htm_s2circle_htmcov.hxx"
#include "_htm_subdivide.hxx"

extern "C" {

int64_t htm_tree_s2circle(const struct htm_tree *tree,
                          const struct htm_v3 *center,
                          double radius,
                          enum htm_errcode *err,
                          htm_callback callback)
{
    struct _htm_path path;
    double d2;
    int64_t count;

    if (tree == NULL || center == NULL) {
        if (err != NULL) {
            *err = HTM_ENULLPTR;
        }
        return -1;
    }
    if (tree->index == MAP_FAILED) {
        return htm_tree_s2circle_scan(tree, center, radius, err, callback);
    } else if (radius < 0.0) {
        /* circle is empty */
        return 0;
    } else if (!callback && radius >= 180.0) {
        /* entire sky */
        return (int64_t) tree->count;
    }

    count = 0;
    /* compute square of secant distance corresponding to radius */
    d2 = sin(radius * 0.5 * HTM_RAD_PER_DEG);
    d2 = 4.0 * d2 * d2;

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

            enum _htm_cov coverage=_htm_s2circle_htmcov(curnode, center, d2);
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

            /* fully covered HTM triangle */
            if(!callback && coverage==HTM_INSIDE)
              {
                count += (int64_t) curcount;
              }
            /* scan points in leaf */
            else if(coverage==HTM_CONTAINS || coverage==HTM_INTERSECT
                    || coverage==HTM_INSIDE)
              {
                uint64_t i;
                for (i = index; i < index + curcount; ++i) {
                  if (coverage==HTM_INSIDE
                      || htm_v3_dist2(center,(struct htm_v3*)
                                      (static_cast<char*>(tree->entries)
                                       + i*tree->entry_size)) <= d2)
                    {
                      if(!callback
                         || callback(static_cast<char*>(tree->entries)
                                     + i*tree->entry_size,
                                     tree->num_elements_per_entry,
                                     tree->element_types,
                                     tree->element_names))
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
    if (err != NULL) {
        *err = HTM_OK;
    }
    return count;
}

}
