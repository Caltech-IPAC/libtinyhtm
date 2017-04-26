#include "htm.hxx"

int _htm_s2ellipse_isect (const struct htm_v3 *v1, const struct htm_v3 *v2,
                          const struct htm_s2ellipse *ellipse);

/*  Returns the coverage code describing the spatial relationship between the
    given HTM triangle and spherical ellipse.
 */
enum _htm_cov _htm_s2ellipse_htmcov (const struct _htm_node *n,
                                     const struct htm_s2ellipse *e)
{
  int nin = htm_s2ellipse_cv3 (e, n->vert[0]);
  nin += htm_s2ellipse_cv3 (e, n->vert[1]);
  nin += htm_s2ellipse_cv3 (e, n->vert[2]);
  if (nin == 3)
    {
      return HTM_INSIDE;
    }
  else if (nin != 0)
    {
      return HTM_INTERSECT;
    }
  /* no triangle vertices inside ellipse - check for edge/ellipse
     intersections */
  if (_htm_s2ellipse_isect (n->vert[0], n->vert[1], e)
      || _htm_s2ellipse_isect (n->vert[1], n->vert[2], e)
      || _htm_s2ellipse_isect (n->vert[2], n->vert[0], e))
    {
      return HTM_INTERSECT;
    }
  /* no triangle/ellipse intersections */
  if (htm_v3_dot (&e->cen, n->edge[0]) >= 0.0
      && htm_v3_dot (&e->cen, n->edge[1]) >= 0.0
      && htm_v3_dot (&e->cen, n->edge[2]) >= 0.0)
    {
      /* ellipse center inside triangle */
      return HTM_CONTAINS;
    }
  return HTM_DISJOINT;
}
