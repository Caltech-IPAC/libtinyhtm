#include "htm.hxx"

/*  Returns 1 if the edge between v1 and v2 intersects the given spherical
    ellipse.

    Let M be the 3x3 symmetric matrix corresponding to the ellipse; the
    ellipse boundary is given by:

    v' M v = 0

    (where ' denotes transpose, and v = [x y z]'). The lines of intersection,
    between the plane defined by v1, v2 and M are given by:

    a*(v1 + v2) + b*(v2 - v1)

    where a,b are scalars to be determined. Note that we can use any basis
    for the plane; (v1 + v2, v2 - v1) is chosen because of numerical stability
    when v1 and v2 are nearly identical, and because it yields a simple test
    for whether a particular solution lies on the edge in question.

    Note that if a,b is a solution, so is k*a,k*b for any scalar k. So
    wlog, set a = 1 and plug into the ellipse boundary equation:

    0 = (v1 + v2)' M (v1 + v2) + b*(v2 - v1)' M (v1 + v2) +
        b*(v1 + v2)' M (v2 - v1) + b^2*(v2 - v1)' M (v2 - v1)

    Since M is symmetric:

    0 = b^2 * (v2 - v1)' M (v2 - v1) +
         2b * (v2 - v1)' M (v1 + v2) +
              (v1 + v2)' M (v1 + v2)

    0 = c22 * b^2 + 2*c21 * b + c11

    Solving this quadratic equation yields 0, 1, or 2 lines of intersection.
    Note that for an intersection to lie on the edge between v1 and v2,
    b must be in the range [-1, 1].
 */
int _htm_s2ellipse_isect (const struct htm_v3 *v1, const struct htm_v3 *v2,
                          const struct htm_s2ellipse *ellipse)
{
  struct htm_v3 e1, e2, v;
  double c11, c21, c22, delta;

  htm_v3_add (&e1, v1, v2);
  htm_v3_sub (&e2, v2, v1);
  /* compute coeffs of quadratic eqn. */
  c11 = e1.x * e1.x * ellipse->xx + e1.y * e1.y * ellipse->yy
        + e1.z * e1.z * ellipse->zz + e1.x * e1.y * ellipse->xy * 2.0
        + e1.x * e1.z * ellipse->xz * 2.0 + e1.y * e1.z * ellipse->yz * 2.0;
  c22 = e2.x * e2.x * ellipse->xx + e2.y * e2.y * ellipse->yy
        + e2.z * e2.z * ellipse->zz + e2.x * e2.y * ellipse->xy * 2.0
        + e2.x * e2.z * ellipse->xz * 2.0 + e2.y * e2.z * ellipse->yz * 2.0;
  c21 = e2.x * e1.x * ellipse->xx + e2.y * e1.y * ellipse->yy
        + e2.z * e1.z * ellipse->zz + (e2.x * e1.y + e2.y * e1.x) * ellipse->xy
        + (e2.x * e1.z + e2.z * e1.x) * ellipse->xz
        + (e2.y * e1.z + e2.z * e1.y) * ellipse->yz;
  if (c11 == 0.0)
    {
      /* v1 + v2 is a solution, and lies on the edge */
      if (ellipse->a >= 90.0 || htm_v3_dot (&e1, &ellipse->cen) >= 0.0)
        {
          return 1;
        }
      /* other solution is given by a linear equation */
      if (c22 == 0.0 || fabs (c22) < fabs (2.0 * c21))
        {
          return 0;
        }
      /* check whether solution lies in correct hemisphere */
      htm_v3_mul (&v, &e2, -2.0 * c21 / c22);
      htm_v3_add (&v, &v, &e1);
      return htm_v3_dot (&v, &ellipse->cen) >= 0.0;
    }
  if (c22 == 0.0)
    {
      /* v2 - v1 is a solution, the other is given by b = -c11/(2*c21). */
      if (c21 == 0.0)
        {
          return 0;
        }
      if (fabs (c11) <= fabs (2.0 * c21))
        {
          if (ellipse->a >= 90.0)
            {
              return 1;
            }
          /* check whether solution lies in correct hemisphere */
          htm_v3_mul (&v, &e2, -0.5 * c11 / c21);
          htm_v3_add (&v, &v, &e1);
          return htm_v3_dot (&v, &ellipse->cen) >= 0.0;
        }
      return 0;
    }
  delta = c21 * c21 - c11 * c22;
  if (delta < 0.0)
    {
      /* no solutions */
      return 0;
    }
  /* 1 or 2 solutions */
  delta = sqrt (delta);
  if (fabs (c22) >= fabs (delta - c21))
    {
      if (ellipse->a >= 90.0)
        {
          return 1;
        }
      /* check whether solution lies in correct hemisphere */
      htm_v3_mul (&v, &e2, (delta - c21) / c22);
      htm_v3_add (&v, &v, &e1);
      return htm_v3_dot (&v, &ellipse->cen) >= 0.0;
    }
  if (fabs (c22) >= fabs (delta + c21))
    {
      if (ellipse->a >= 90.0)
        {
          return 1;
        }
      /* check whether solution lies in correct hemisphere */
      htm_v3_mul (&v, &e2, -(delta + c21) / c22);
      htm_v3_add (&v, &v, &e1);
      return htm_v3_dot (&v, &ellipse->cen) >= 0.0;
    }
  return 0;
}
