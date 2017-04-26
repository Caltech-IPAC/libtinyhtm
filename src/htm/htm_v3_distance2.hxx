#pragma once

#include "tinyhtm/htm.h"

template <typename T>
inline double htm_v3_distance2 (const struct htm_v3 *v1, const T *v2)
{
  htm_v3 temp;
  temp.x = v2[0];
  temp.y = v2[1];
  temp.z = v2[2];
  return htm_v3_dist2 (v1, &temp);
}

template <>
inline double htm_v3_distance2 (const struct htm_v3 *v1, const double *v2)
{
  return htm_v3_dist2 (v1, reinterpret_cast<const struct htm_v3 *>(v2));
}
