#include "htm.hxx"

/*  Partitions an array of points according to their root triangle numbers.
 */
size_t _htm_rootpart (struct htm_v3p *points, unsigned char *ids, size_t n,
                      enum htm_root root)
{
  struct htm_v3p tmp;
  size_t beg, end;
  unsigned char c;
  for (beg = 0, end = n; beg < end; ++beg)
    {
      if (ids[beg] >= root)
        {
          for (; end > beg; --end)
            {
              if (ids[end - 1] < root)
                {
                  break;
                }
            }
          if (end == beg)
            {
              break;
            }
          tmp = points[beg];
          points[beg] = points[end - 1];
          points[end - 1] = tmp;
          c = ids[beg];
          ids[beg] = ids[end - 1];
          ids[end - 1] = c;
        }
    }
  return beg;
}
