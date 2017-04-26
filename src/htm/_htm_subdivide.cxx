#include "tinyhtm/varint.h"
#include "htm.hxx"

/*  Constructs the next non-empty child of \p node.
 */
const unsigned char *_htm_subdivide (struct _htm_node *node,
                                     const unsigned char *s)
{
  uint64_t off;
  switch (node->child)
    {
    case 0:
      off = htm_varint_decode (s);
      s += 1 + htm_varint_nfollow (*s);
      _htm_node_prep0 (node);
      _htm_node_make0 (node);
      if (off != 0)
        {
          break;
        }
    /* fall-through */
    case 1:
      off = htm_varint_decode (s);
      s += 1 + htm_varint_nfollow (*s);
      _htm_node_prep1 (node);
      _htm_node_make1 (node);
      if (off != 0)
        {
          break;
        }
    /* fall-through */
    case 2:
      off = htm_varint_decode (s);
      s += 1 + htm_varint_nfollow (*s);
      _htm_node_prep2 (node);
      _htm_node_make2 (node);
      if (off != 0)
        {
          break;
        }
    /* fall-through */
    case 3:
      off = htm_varint_decode (s);
      s += 1 + htm_varint_nfollow (*s);
      if (off != 0)
        {
          _htm_node_make3 (node);
          break;
        }
    default:
      return NULL;
    }
  node->s = s;
  return s + (off - 1);
}
