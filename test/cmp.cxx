/** \file
    \brief      floating point comparison implementation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */
#include "cmp.h"

#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif


static double safe_div(double n, double d)
{
    if (d < 1.0 && n > d * DBL_MAX) {
        return DBL_MAX; /* overflow to DBL_MAX */
    }
    if (n == 0.0 || (d > 1.0 && n < d * DBL_MIN)) {
        return 0.0; /* underflow to zero */
    }
    return n / d;
}

int almost_equal(double u, double v, double tolerance)
{
    double delta = fabs(u - v);
    double eps = fabs(tolerance);
    u = safe_div(delta, fabs(u));
    v = safe_div(delta, fabs(v));
    return u <= eps && v <= eps;
}

#ifdef __cplusplus
}
#endif

