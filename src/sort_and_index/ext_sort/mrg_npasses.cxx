/*  Computes the number of k-way merge passes required to sort n items,
    i.e. the ceiling of the base-k logarithm of n.
 */

#include <cstddef>

int mrg_npasses (size_t n, size_t k)
{
  int m = 1;
  size_t b = k;
  while (b < n)
    {
      b *= k;
      ++m;
    }
  return m;
}
