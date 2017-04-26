/*  Writes out a tree header (byte reversed).
 */

#include <stdexcept>
#include "tinyhtm.h"
#include "../../tinyhtm/varint.h"
#include "hash_table.hxx"
#include "../blk_writer.hxx"
#include "../tree_root.hxx"

uint64_t write_tree_header (struct hash_table *const ht,
                            blk_writer<unsigned char, false> &wr,
                            const tree_root &super, const uint64_t filesz,
                            const uint64_t leafthresh)
{
  unsigned char buf[96];
  unsigned char *s;
  uint64_t sz;
  int r;
  unsigned int v;

  /* write offsets of N3, N2, N1, N0, S3, S2, S1, S0 */
  for (r = 7, s = buf, sz = filesz; r >= 0; --r)
    {
      if (node_empty (&super.childid[r]))
        {
          *s = 0;
          ++s;
          ++sz;
        }
      else
        {
          /* N3 could be laid out immediately after super root,
             yielding a child offset of 0, which means "empty child".
             Therefore encode 1 + actual offset. */
          v = htm_varint_rencode (s, sz + 1
                                     - hash_table_get (ht, &super.childid[r]));
          s += v;
          sz += v;
        }
    }
  /* write total number of points in tree */
  v = htm_varint_rencode (s, super.count);
  s += v;
  sz += v;
  /* write leaf threshold */
  v = htm_varint_rencode (s, leafthresh);
  s += v;
  sz += v;
  wr.append (buf, (size_t)(s - buf));
  if (ht->n != 0)
    {
      throw std::runtime_error (
          "tree compression bug: node id hash table non-empty");
    }
  return sz;
}
