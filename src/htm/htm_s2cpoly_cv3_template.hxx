#pragma once

#include "tinyhtm/htm.h"

template <typename T>
inline double htm_s2cpoly_cv3_template (const struct htm_s2cpoly *p,
                                        const T *v2)
{
  htm_v3 temp;
  temp.x = v2[0];
  temp.y = v2[1];
  temp.z = v2[2];
  return htm_s2cpoly_cv3 (p, &temp);
}

template <>
inline double htm_s2cpoly_cv3_template (const struct htm_s2cpoly *p,
                                        const double *v2)
{
  return htm_s2cpoly_cv3 (p, reinterpret_cast<const struct htm_v3 *>(v2));
}
