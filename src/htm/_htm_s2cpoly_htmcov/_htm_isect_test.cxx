/*  Tests whether poly intersects the edge (v1, v2) with plane normal n.

    The idea is that a solution v = (x,y,z) must satisfy:

        v . n = 0, v != 0
        v . (n ^ v1) >= 0
        v . (v2 ^ n) >= 0
        v . e_i >= 0

    where e_i are the edge plane normals for the polygon, and (n ^ v1),
    (v2 ^ n) are plane normals that bound the lune defined by n, v1, and v2.
    Write this as:

        v . n = 0
        v . c_i >= 0

    Now assume nz > 0 (for a non zero solution, at least one of nx, ny, nz
    must be non-zero, and treatment of negative values is analagous). Use
    the equality to obtain

        z = - (x * nx + y * ny) / nz

    and substitute into the inequalities to obtain:

        x * (c_ix * nz - c_iz * nx) + y * (c_iy * nz - c_iz * ny) >= 0

    which we write

        x * a_i + y * b_i >= 0

    If a solution v exists, then Kv is also a solution (for positive scalar K),
    so we can fix y = 1 and look for solutions to

        x * a_i + b_i >= 0

    If there are none, fix y = -1 and look for solutions to:

        x * a_i - b_i >= 0

    If again there are none, then y = 0, and the problem reduces to checking
    whether

        x * a_i >= 0

    has any solutions. This is the case when the non-zero a_i have the same
    sign.
 */

#include "htm.hxx"

int _htm_isect_test (const struct htm_v3 *v1, const struct htm_v3 *v2,
                     const struct htm_v3 *n, const struct htm_s2cpoly *poly,
                     double *ab)
{
  struct htm_v3 c0;
  struct htm_v3 c1;
  double min_1, max_1, min_m1, max_m1;
  size_t nv, i, neg, pos;

  htm_v3_cross (&c0, n, v1);
  htm_v3_cross (&c1, v2, n);
  nv = poly->n;
  if (n->z != 0.0)
    {
      double s = (n->z > 0.0) ? 1.0 : -1.0;
      ab[0] = s * (c0.x * n->z - c0.z * n->x);
      ab[1] = s * (c0.y * n->z - c0.z * n->y);
      ab[2] = s * (c1.x * n->z - c1.z * n->x);
      ab[3] = s * (c1.y * n->z - c1.z * n->y);
      for (i = 0; i < nv; ++i)
        {
          ab[2 * i + 4]
              = s * (poly->ve[nv + i].x * n->z - poly->ve[nv + i].z * n->x);
          ab[2 * i + 5]
              = s * (poly->ve[nv + i].y * n->z - poly->ve[nv + i].z * n->y);
        }
    }
  else if (n->y != 0.0)
    {
      double s = (n->y > 0.0) ? 1.0 : -1.0;
      ab[0] = s * (c0.x * n->y - c0.y * n->x);
      ab[1] = s * (c0.z * n->y);
      ab[2] = s * (c1.x * n->y - c1.y * n->x);
      ab[3] = s * (c1.z * n->y);
      for (i = 0; i < nv; ++i)
        {
          ab[2 * i + 4]
              = s * (poly->ve[nv + i].x * n->y - poly->ve[nv + i].y * n->x);
          ab[2 * i + 5] = s * (poly->ve[nv + i].z * n->y);
        }
    }
  else if (n->x != 0.0)
    {
      double s = (n->x > 0.0) ? 1.0 : -1.0;
      ab[0] = s * (c0.y * n->x);
      ab[1] = s * (c0.z * n->x);
      ab[2] = s * (c1.y * n->x);
      ab[3] = s * (c1.z * n->x);
      for (i = 0; i < nv; ++i)
        {
          ab[2 * i + 4] = s * (poly->ve[nv + i].y * n->x);
          ab[2 * i + 5] = s * (poly->ve[nv + i].z * n->x);
        }
    }
  else
    {
      return 0;
    }
  /* search for solutions to a*x +/- b >= 0, with constraint coeffs stored in
     ab */

  // FIXME: This may get optimized out.  Maybe we should use
  // numeric_limits<>::max?
  const double HTM_INF = 1.0 / 0.0;
  const double HTM_NEG_INF = -1.0 / 0.0;

  min_1 = min_m1 = HTM_NEG_INF;
  max_1 = max_m1 = HTM_INF;
  for (i = 0, neg = 0, pos = 0; i < nv + 2; ++i)
    {
      double a = ab[2 * i];
      double b = ab[2 * i + 1];
      if (a == 0.0)
        {
          if (b < 0.0)
            {
              min_1 = HTM_INF;
              max_1 = HTM_NEG_INF;
            }
          else if (b > 0.0)
            {
              min_m1 = HTM_INF;
              max_m1 = HTM_NEG_INF;
            }
        }
      else if (a < 0.0)
        {
          ++neg;
          double d = -b / a;
          if (d < max_1)
            {
              max_1 = d;
            }
          if (-d < max_m1)
            {
              max_m1 = -d;
            }
        }
      else
        {
          ++pos;
          double d = -b / a;
          if (d > min_1)
            {
              min_1 = d;
            }
          if (-d > min_m1)
            {
              min_m1 = -d;
            }
        }
    }
  if (min_1 <= max_1 || min_m1 <= max_m1)
    {
      return 1;
    }
  return (neg == 0 || pos == 0);
}
