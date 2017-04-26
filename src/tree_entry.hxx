#ifndef HTM_TREE_GEN_TREE_ENTRY_H
#define HTM_TREE_GEN_TREE_ENTRY_H

#include <array>
#include <cstdint>
#include <cassert>

#include <H5Cpp.h>
#include "tinyhtm.h"

/*  An entry in an HTM tree.
 */
struct tree_entry
{
  int64_t htmid;
  int64_t rowid;
  struct htm_sc sc;

  typedef double vector_type;
  static constexpr size_t data_size = 8;
  static std::array<std::string, 1> names;
  static std::array<H5::DataType, 1> types;

  const void *data (const size_t) const { return &rowid; }

  void *data (const size_t) { return &rowid; }

  /*  Tests whether tree entry e1 is less than e2; this is the same as
      testing whether the HTM ID of e1 is less than e2.  Row IDs are
      used to break ties. */
  bool operator<(const tree_entry &e) const
  {
    return (htmid < e.htmid || (htmid == e.htmid && rowid < e.rowid));
  }
} HTM_ALIGNED (16);

#endif
