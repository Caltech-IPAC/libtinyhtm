/** \file
    \brief      Utilities for comparing floating point numbers and points.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */
#ifndef HTM_CMP_H
#define HTM_CMP_H

#include "tinyhtm/geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

int almost_equal(double u, double v, double tolerance);

HTM_INLINE int v3_equal(const struct htm_v3 *v1, const struct htm_v3 *v2)
{
    return v1->x == v2->x &&
           v1->y == v2->y &&
           v1->z == v2->z;
}

HTM_INLINE int v3_almost_equal(const struct htm_v3 *v1,
                               const struct htm_v3 *v2,
                               double tolerance)
{
    return almost_equal(htm_v3_dot(v1, v2),
                        htm_v3_norm(v1) * htm_v3_norm(v2),
                        tolerance);
}

HTM_INLINE int sc_equal(const struct htm_sc *p1, const struct htm_sc *p2)
{
    return p1->lon == p2->lon && p1->lat == p2->lat;
}

HTM_INLINE int sc_almost_equal(const struct htm_sc *p1,
                               const struct htm_sc *p2,
                               double tolerance)
{
    return almost_equal(p1->lon, p2->lon, tolerance) &&
           almost_equal(p1->lat, p2->lat, tolerance);
}

#ifdef __cplusplus
}
#endif

#endif /* HTM_CMP_H */

