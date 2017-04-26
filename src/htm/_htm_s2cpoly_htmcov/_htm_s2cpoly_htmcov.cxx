/*  Returns the coverage code describing the spatial relationship between the
    given HTM triangle and spherical convex polygon.
 */

#include "htm.hxx"

int _htm_isect_test (const struct htm_v3 *v1, const struct htm_v3 *v2,
                     const struct htm_v3 *n, const struct htm_s2cpoly *poly,
                     double *ab);

enum _htm_cov _htm_s2cpoly_htmcov (const struct _htm_node *n,
                                   const struct htm_s2cpoly *poly, double *ab)
{
  int nin = htm_s2cpoly_cv3 (poly, n->vert[0]);
  nin += htm_s2cpoly_cv3 (poly, n->vert[1]);
  nin += htm_s2cpoly_cv3 (poly, n->vert[2]);

  if (nin == 3)
    {
      /* all triangle vertices are inside poly,
         so triangle is inside by convexity. */
      return HTM_INSIDE;
    }
  else if (nin != 0)
    {
      return HTM_INTERSECT;
    }
  /* all triangle vertices are outside poly - check for edge intersections */
  if (_htm_isect_test (n->vert[0], n->vert[1], n->edge[0], poly, ab) != 0
      || _htm_isect_test (n->vert[1], n->vert[2], n->edge[1], poly, ab) != 0
      || _htm_isect_test (n->vert[2], n->vert[0], n->edge[2], poly, ab) != 0)
    {
      return HTM_INTERSECT;
    }
  /* All triangle vertices are outside poly and there are no edge/edge
     intersections. Polygon is either inside triangle or disjoint from
     it */
  if (htm_v3_dot (&poly->vsum, n->edge[0]) >= 0.0
      && htm_v3_dot (&poly->vsum, n->edge[1]) >= 0.0
      && htm_v3_dot (&poly->vsum, n->edge[2]) >= 0.0)
    {
      return HTM_CONTAINS;
    }
  return HTM_DISJOINT;
}
