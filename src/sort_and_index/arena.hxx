#ifndef HTM_TREE_GEN_ARENA_H
#define HTM_TREE_GEN_ARENA_H

#include "tinyhtm.h"
#include "node.hxx"
#include "arena/arena_seg.hxx"

/*  A (non-shrinkable) memory arena. Memory nodes can be freed one
    at a time, or en-masse.
 */
struct arena
{
  struct arena_seg *tail; /* reverse linked list of memory segments */
  struct mem_node *head;  /* head of linked list of free memory locations */
  size_t itemsz;
  size_t nseg;

  arena () = delete;
  arena (const size_t Itemsz)
      : tail (new arena_seg (nullptr, Itemsz)),
        head (static_cast<mem_node *>(tail->mem)), itemsz (Itemsz), nseg (1)
  {
  }

  void *alloc ()
  {
    void *item;
    if (head == nullptr)
      {
        tail = new arena_seg (tail, itemsz);
        head = static_cast<mem_node *>(tail->mem);
        ++nseg;
      }
    item = head;
    head = static_cast<mem_node *>(*((void **)item));
    return item;
  }

  void free (void *const n)
  {
    *((void **)n) = head;
    head = static_cast<mem_node *>(n);
  }
  ~arena ()
  {
    arena_seg *seg = tail;
    while (seg != nullptr)
      {
        arena_seg *prev = seg->prev;
        delete (seg);
        seg = prev;
      }
  }
};

struct arena_seg *arena_seg_init (struct arena_seg *const prev,
                                  const size_t itemsz);

#endif
