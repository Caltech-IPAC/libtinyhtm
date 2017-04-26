#include "htm.hxx"

/*  Reorders the vectors in [begin,end) such that the resulting array can be
    partitioned into [begin,m) and [m,end), where all vectors in [begin,m) are
    inside the partitioning plane, and all vectors in [m, end) are outside.

    A pointer to the partitioning element m is returned.

    Assumes that plane != 0, begin != 0, end != 0, and begin <= end.
 */
struct htm_v3p *_htm_partition (const struct htm_v3 *plane,
                                struct htm_v3p *beg, struct htm_v3p *end)
{
  struct htm_v3p tmp;
  for (; beg < end; ++beg)
    {
      if (htm_v3_dot (plane, &beg->v) < 0.0)
        {
          /* beg is outside plane, find end which is inside,
             swap contents of beg and end. */
          for (--end; end > beg; --end)
            {
              if (htm_v3_dot (plane, &end->v) >= 0.0)
                {
                  break;
                }
            }
          if (end <= beg)
            {
              break;
            }
          tmp = *beg;
          *beg = *end;
          *end = tmp;
        }
    }
  return beg;
}
