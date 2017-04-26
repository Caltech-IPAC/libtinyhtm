/*  Writes out a tree node (byte reversed).
 */

#include <stdexcept>
#include "../../tinyhtm/varint.h"
#include "hash_table.hxx"
#include "../blk_writer.hxx"

uint64_t compress_node (struct hash_table *const ht,
                        blk_writer<unsigned char, false> &wr,
                        const struct disk_node *const n, const uint64_t filesz,
                        const uint64_t leafthresh)
{
  unsigned char buf[48];
  unsigned char *s = buf;
  uint64_t sz = filesz;
  unsigned int v;
  int c, leaf;

  /* write out child offsets (from child 3 to child 0) */
  for (c = 3, leaf = 1; c >= 0; --c)
    {
      if (node_empty (&n->child[c]))
        {
          *s = 0;
          ++s;
          ++sz;
        }
      else
        {
          /* this is tricky - child 3 of n can be laid out immediately after
             n, yielding a child offset of 0. But 0 also means "empty child",
             so instead encode the actual offset + 1. */
          v = htm_varint_rencode (s,
                                  sz + 1 - hash_table_get (ht, &n->child[c]));
          s += v;
          sz += v;
          leaf = 0;
        }
    }
  if (leaf != 0)
    {
      /* n is a leaf: don't store child offsets */
      s -= 4;
      sz -= 4;
    }
  else if (n->count < leafthresh)
    {
      throw std::runtime_error (
          "tree generation bug: internal node contains too few points");
    }
  /* write out relative index, then count */
  v = htm_varint_rencode (s, n->index);
  s += v;
  sz += v;
  v = htm_varint_rencode (s, n->count);
  s += v;
  sz += v;
  /* write out byte reversed node, add node id to hashtable */
  wr.append (buf, (size_t)(s - buf));
  hash_table_add (ht, &n->id, sz);
  return sz;
}
