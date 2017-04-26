/*  Sorts the given array of positions by root triangle number.
 */

#include "htm/_htm_v3_htmroot.hxx"

size_t _htm_rootpart (struct htm_v3p *points, unsigned char *ids, size_t n,
                      enum htm_root root);

void _htm_rootsort (size_t roots[HTM_NROOTS + 1], struct htm_v3p *points,
                    unsigned char *ids, size_t n)
{
  size_t i, n0, n2, s2;

  /* compute root ids for all points */
  for (i = 0; i < n; ++i)
    {
      ids[i] = (unsigned char)_htm_v3_htmroot (&points[i].v);
    }
  n0 = _htm_rootpart (points, ids, n, HTM_N0);
  s2 = _htm_rootpart (points, ids, n0, HTM_S2);
  roots[HTM_S0] = 0;
  roots[HTM_S1] = _htm_rootpart (points, ids, s2, HTM_S1);
  roots[HTM_S2] = s2;
  roots[HTM_S3] = _htm_rootpart (points + s2, ids + s2, n0 - s2, HTM_S3) + s2;
  n2 = _htm_rootpart (points + n0, ids + n0, n - n0, HTM_N2) + n0;
  roots[HTM_N0] = n0;
  roots[HTM_N1] = _htm_rootpart (points + n0, ids + n0, n2 - n0, HTM_N1) + n0;
  roots[HTM_N2] = n2;
  roots[HTM_N3] = _htm_rootpart (points + n2, ids + n2, n - n2, HTM_N3) + n2;
  roots[HTM_NROOTS] = n;
}
