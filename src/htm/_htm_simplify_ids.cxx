#include "htm.hxx"

/*  Reduces the effective subdivision level of an HTM id range list by n levels
    and merges adjacent ranges. This typically reduces the number of ranges in
    the list, but also makes it a poorer approximation of the underlying
    geometry. Note that with sufficiently large n, any range list can be shrunk
    down to at most 4 ranges.

    In detail: a range [I1, I2] is mapped to [I1 & ~mask, I2 | mask], where
    mask = (1 << 2*n) - 1.
 */
void _htm_simplify_ids (struct htm_ids *ids, int n)
{
  size_t i, j, nr;
  int64_t mask;
  if (n <= 0 || ids == 0 || ids->n == 0)
    {
      return;
    }
  mask = (((int64_t)1) << 2 * n) - 1;
  for (i = 0, j = 0, nr = ids->n; i < nr; ++i, ++j)
    {
      int64_t min = ids->range[i].min & ~mask;
      int64_t max = ids->range[i].max | mask;
      for (; i < nr - 1; ++i)
        {
          int64_t next = ids->range[i + 1].min & ~mask;
          if (next > max + 1)
            {
              break;
            }
          max = ids->range[i + 1].max | mask;
        }
      ids->range[j].min = min;
      ids->range[j].max = max;
    }
  ids->n = j;
}
