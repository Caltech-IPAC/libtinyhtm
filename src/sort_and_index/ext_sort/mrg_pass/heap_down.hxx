#ifndef SORT_AND_INDEX_HEAP_DOWN_HXX
#define SORT_AND_INDEX_HEAP_DOWN_HXX

/*  Fix possible min-heap violation by segs[0].
 */

#include "../mrg_seg.hxx"

template <class T> void heap_down (mrg_seg<T> *segs, const size_t n)
{
  mrg_seg<T> tmp;
  size_t i;

  if (n > 1)
    {
      i = 0;
      while (1)
        {
          size_t left = 2 * i + 1;
          size_t right = 2 * i + 2;
          size_t least = i;
          if (left < n && *(segs[left].cur) < *(segs[i].cur))
            {
              least = left;
            }
          if (right < n && *(segs[right].cur) < *(segs[least].cur))
            {
              least = right;
            }
          if (least == i)
            {
              break;
            }
          tmp = segs[i];
          segs[i] = segs[least];
          segs[least] = tmp;
          i = least;
        }
    }
}

#endif
