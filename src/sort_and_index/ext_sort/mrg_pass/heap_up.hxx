#ifndef SORT_AND_INDEX_HEAP_UP_HXX
#define SORT_AND_INDEX_HEAP_UP_HXX

/*  Adds segs[n] to the min-heap segs[0], segs[1], ..., segs[n - 1].
 */

#include "../mrg_seg.hxx"

template <class T> void heap_up (mrg_seg<T> *segs, size_t n)
{
  mrg_seg<T> tmp;
  size_t p;

  while (n != 0)
    {
      p = (n - 1) / 2;
      if (*(segs[p].cur) < *(segs[n].cur))
        {
          break;
        }
      tmp = segs[p];
      segs[p] = segs[n];
      segs[n] = tmp;
      n = p;
    }
}
#endif
