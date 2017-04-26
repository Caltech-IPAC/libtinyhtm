#include "htm.hxx"

/*  Returns the coverage code describing the spatial relationship between the
    given HTM triangle and spherical circle.
 */
enum _htm_cov _htm_s2circle_htmcov (const struct _htm_node *n,
                                    const struct htm_v3 *c, double dist2)
{
  int nin = htm_v3_dist2 (c, n->vert[0]) <= dist2;
  nin += htm_v3_dist2 (c, n->vert[1]) <= dist2;
  nin += htm_v3_dist2 (c, n->vert[2]) <= dist2;
  if (nin == 3)
    {
      /* every vertex inside circle */
      return HTM_INSIDE;
    }
  else if (nin != 0)
    {
      return HTM_INTERSECT;
    }
  /* no triangle vertices inside circle */
  if (htm_v3_edgedist2 (c, n->vert[0], n->vert[1], n->edge[0]) <= dist2
      || htm_v3_edgedist2 (c, n->vert[1], n->vert[2], n->edge[1]) <= dist2
      || htm_v3_edgedist2 (c, n->vert[2], n->vert[0], n->edge[2]) <= dist2)
    {
      /* min distance to at least one edge is <= circle radius */
      return HTM_INTERSECT;
    }
  /* min distance to every edge is > circle radius - circle
     is either inside triangle or disjoint from it */
  if (htm_v3_dot (c, n->edge[0]) >= 0.0 && htm_v3_dot (c, n->edge[1]) >= 0.0
      && htm_v3_dot (c, n->edge[2]) >= 0.0)
    {
      return HTM_CONTAINS;
    }
  return HTM_DISJOINT;
}
