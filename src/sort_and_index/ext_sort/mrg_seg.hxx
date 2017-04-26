#ifndef HTM_TREE_GEN_MRG_SEQ_H
#define HTM_TREE_GEN_MRG_SEQ_H

#include <unistd.h>
#include <sys/mman.h>
#include "tinyhtm.h"

/*  A contiguous sequence of sorted items, dubbed a multi-way merge segment.
 */
template <class T> struct mrg_seg
{
  T *cur;
  T *end;
  T *blk;

  void init (T *Cur, T *End, const size_t blksz)
  {
    size_t n;
    const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
    size_t nbytes = (End - Cur) * sizeof(T);
    if (End <= Cur || blksz % pagesz != 0)
      {
        throw std::runtime_error ("Invalid merge segment");
      }
    cur = Cur;
    end = End;
    n = reinterpret_cast<size_t>(Cur) % pagesz;
    if (n != 0)
      {
        Cur = reinterpret_cast<T *>(reinterpret_cast<char *>(Cur) - n);
        nbytes += n;
      }
    n = (nbytes > 2 * blksz ? 2 * blksz : nbytes);
    if (n % pagesz != 0)
      {
        n += pagesz - n % pagesz;
      }
    blk = reinterpret_cast<T *>(reinterpret_cast<char *>(Cur) + blksz);
    if (madvise ((void *)Cur, n, MADV_WILLNEED) != 0)
      {
        throw std::runtime_error ("madvise() failed");
      }
  }

  int advance (const size_t blksz, T *Cur)
  {
    const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);

    if (Cur == end)
      {
        void *start = reinterpret_cast<char *>(blk) - blksz;
        size_t n = reinterpret_cast<char *>(end)
                   - reinterpret_cast<char *>(start);
        if (n % pagesz != 0)
          {
            n += pagesz - n % pagesz;
          }
        if (madvise (start, n, MADV_DONTNEED) != 0)
          {
            throw std::runtime_error ("madvise() failed");
          }
        return 0;
      }
    assert (Cur >= blk && Cur < end);
    if (madvise (reinterpret_cast<char *>(blk) - blksz, blksz, MADV_DONTNEED)
        != 0)
      {
        throw std::runtime_error ("madvise() failed");
      }
    cur = Cur;
    blk = reinterpret_cast<T *>(reinterpret_cast<char *>(blk) + blksz);
    if (blk < end)
      {
        size_t n = reinterpret_cast<char *>(end)
                   - reinterpret_cast<char *>(blk);
        if (n >= blksz)
          {
            n = blksz;
          }
        else if (n % pagesz != 0)
          {
            n += pagesz - n % pagesz;
          }
        if (madvise ((void *)blk, n, MADV_WILLNEED) != 0)
          {
            throw std::runtime_error ("madvise() failed");
          }
      }
    return 1;
  }

  int consume (const size_t blksz, const size_t itemsz)
  {
    T *Cur (reinterpret_cast<T *>(reinterpret_cast<char *>(cur) + itemsz));
    if (Cur < end && Cur < blk)
      {
        cur = Cur;
        return 1;
      }
    return advance (blksz, Cur);
  }
};

// void mrg_seg_init(struct mrg_seg * const s,
//                   const void * start,
//                   const void * end,
//                   const size_t blksz);

// int mrg_seg_advance(struct mrg_seg * const s,
//                     const size_t blksz,
//                     const void * const cur);

// HTM_INLINE int mrg_seg_consume(struct mrg_seg * const s,
//                                const size_t blksz,
//                                const size_t itemsz)
// {
//   const void *cur = (const char *) s->cur + itemsz;
//   if (cur < s->end && cur < s->blk) {
//     s->cur = cur;
//     return 1;
//   }
//   return mrg_seg_advance(s, blksz, cur);
// }

#endif
