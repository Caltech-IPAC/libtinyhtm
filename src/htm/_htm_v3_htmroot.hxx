#pragma once

#include "htm.hxx"

/*  Returns the HTM root triangle for a point.
 */
inline enum htm_root _htm_v3_htmroot (const struct htm_v3 *v)
{
  if (v->z < 0.0)
    {
      /* S0, S1, S2, S3 */
      if (v->y > 0.0)
        {
          return (v->x > 0.0) ? HTM_S0 : HTM_S1;
        }
      else if (v->y == 0.0)
        {
          return (v->x >= 0.0) ? HTM_S0 : HTM_S2;
        }
      else
        {
          return (v->x < 0.0) ? HTM_S2 : HTM_S3;
        }
    }
  else
    {
      /* N0, N1, N2, N3 */
      if (v->y > 0.0)
        {
          return (v->x > 0.0) ? HTM_N3 : HTM_N2;
        }
      else if (v->y == 0.0)
        {
          return (v->x >= 0.0) ? HTM_N3 : HTM_N1;
        }
      else
        {
          return (v->x < 0.0) ? HTM_N1 : HTM_N0;
        }
    }
}
