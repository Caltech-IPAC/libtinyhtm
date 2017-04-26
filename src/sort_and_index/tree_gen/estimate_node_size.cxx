/*  Estimates the compressed on-disk size of a tree node.
 */

#include "../../tinyhtm/varint.h"
#include "../node.hxx"

uint32_t estimate_node_size (const struct mem_node *const node,
                             const uint32_t nchild)
{
  uint32_t sz = htm_varint_len (node->index) + htm_varint_len (node->count);
  if (nchild > 0)
    {
      /* There is no way to compute size of a child offset accurately
         without knowing the final node layout, so use a guess of 4 bytes
         per non-empty child. The offset for an empty child (0) will
         will occupy exactly 1 byte, regardless of layout. */
      sz += nchild * 3 + 4;
    }
  return sz;
}
