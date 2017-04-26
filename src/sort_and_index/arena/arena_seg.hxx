#ifndef HTM_TREE_GEN_ARENA_SEG_H
#define HTM_TREE_GEN_ARENA_SEG_H

#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <sys/mman.h>

#define ARENA_SEGSZ 16 * 1024 * 1024

/*  A contiguous memory segment belonging to an arena.
 */
struct arena_seg
{
  arena_seg *prev;
  void *mem;

  arena_seg () {}
  arena_seg (arena_seg *const Prev, const size_t itemsz)
      : prev (Prev), mem (std::calloc (1, ARENA_SEGSZ))
  {
    if (mem == NULL)
      {
        throw std::runtime_error ("calloc() failed");
      }
    /* initialize free list */
    unsigned char *node = (unsigned char *)mem;
    unsigned char *end = node + (ARENA_SEGSZ / itemsz - 1) * itemsz;
    while (node < end)
      {
        *((void **)node) = node + itemsz;
        node += itemsz;
      }
    *((void **)node) = nullptr;
  }

  ~arena_seg () { free (mem); }
};

#endif
