#include "htm.hxx"

void _htm_path_sort (struct _htm_path *path, struct htm_v3p *begin,
                     struct htm_v3p *end, int64_t *ids, int level);

void _htm_rootsort (size_t roots[HTM_NROOTS + 1], struct htm_v3p *points,
                    unsigned char *ids, size_t n);

extern "C" {

enum htm_errcode htm_v3p_idsort (struct htm_v3p *points, int64_t *ids,
                                 size_t n, int level)
{
  struct _htm_path path;
  size_t roots[HTM_NROOTS + 1];
  int r;

  if (n == 0)
    {
      return HTM_ELEN;
    }
  else if (points == 0 || ids == 0)
    {
      return HTM_ENULLPTR;
    }
  else if (level < 0 || level > HTM_MAX_LEVEL)
    {
      return HTM_ELEVEL;
    }
  _htm_rootsort (roots, points, (unsigned char *)ids, n);
  for (r = HTM_S0; r <= HTM_N3; ++r)
    {
      if (roots[r] < roots[r + 1])
        {
          _htm_path_root (&path, static_cast<htm_root>(r));
          _htm_path_sort (&path, points + roots[r], points + roots[r + 1],
                          ids + roots[r], level);
        }
    }
  return HTM_OK;
}
}
