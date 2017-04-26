#pragma once

#include "Shape.hxx"

namespace tinyhtm
{
class Ellipse : public Shape
{
public:
  struct htm_s2ellipse ellipse;
  Ellipse (const Spherical &s, const double &a, const double &b,
           const double &angle)
  {
    Cartesian c (s);
    enum htm_errcode ec = htm_s2ellipse_init2 (&ellipse, &(c.v3), a, b, angle);
    if (ec != HTM_OK)
      {
        std::stringstream ss;
        ss << "Bad coordinates for initializing tinyhtm::Ellipse: " << s << " "
           << a << " " << b << " " << angle << " " << htm_errmsg (ec);
        throw Exception (ss.str ());
      }
  }

  std::vector<htm_range>
  covering_ranges (const size_t &level,
                   const size_t &max_ranges) const override
  {
    struct htm_ids *ids = nullptr;
    enum htm_errcode ec;
    ids = htm_s2ellipse_ids (ids, &ellipse, level, max_ranges, &ec);
    if (ec != HTM_OK)
      {
        if (ids != nullptr)
          free (ids);
        throw Exception (
            std::string ("Failed to find HTM triangles overlapping ellipse: ")
            + htm_errmsg (ec));
      }
    std::vector<htm_range> ranges;
    ranges.reserve (ids->n);
    for (size_t r = 0; r < ids->n; ++r)
      ranges.push_back (ids->range[r]);
    free (ids);
    return ranges;
  }
  double x () const { return ellipse.cen.x; }
  double y () const { return ellipse.cen.y; }
  double z () const { return ellipse.cen.z; }
  double xx () const { return ellipse.xx; }
  double xy () const { return ellipse.xy; }
  double xz () const { return ellipse.xz; }
  double yy () const { return ellipse.yy; }
  double yz () const { return ellipse.yz; }
  double zz () const { return ellipse.zz; }
};
}
