#pragma once

template <typename T>
inline int htm_s2ellipse_cv3_template (const struct htm_s2ellipse *e,
                                       const T *v)
{
  htm_v3 temp;
  temp.x = v[0];
  temp.y = v[1];
  temp.z = v[2];
  return htm_s2ellipse_cv3 (e, &temp);
}

template <>
inline int htm_s2ellipse_cv3_template (const struct htm_s2ellipse *e,
                                       const double *v)
{
  return htm_s2ellipse_cv3 (e, reinterpret_cast<const struct htm_v3 *>(v));
}
