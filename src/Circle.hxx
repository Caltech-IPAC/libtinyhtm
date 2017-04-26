#pragma once

#include <utility>

#include "Spherical.hxx"
#include "Cartesian.hxx"
#include "Shape.hxx"

namespace tinyhtm
{
class Circle : public Shape
{
public:
  Spherical center;
  double r;

  Circle (const Spherical &Center, const double &R) : center (Center), r (R) {}

  int64_t search (const Tree &tree, htm_callback callback) const override
  {
    Cartesian c (center);
    enum htm_errcode ec;
    int64_t count
        = htm_tree_s2circle (&(tree.tree), &(c.v3), r, &ec, callback);
    if (ec != HTM_OK)
      throw Exception ("Corrupted index file");
    return count;
  }

  std::pair<Spherical, Spherical> bounding_box () const override
  {
    return std::make_pair (center, Spherical (2 * r, 2 * r));
  }

  std::vector<htm_range>
  covering_ranges (const size_t &level,
                   const size_t &max_ranges) const override
  {
    Cartesian cartesian (center);
    struct htm_ids *ids = nullptr;
    enum htm_errcode ec;
    ids = htm_s2circle_ids (ids, &(cartesian.v3), r, level, max_ranges, &ec);
    if (ec != HTM_OK)
      {
        if (ids != nullptr)
          free (ids);
        throw Exception (
            std::string ("Failed to find HTM triangles overlapping circle: ")
            + htm_errmsg (ec));
      }
    std::vector<htm_range> ranges;
    ranges.reserve (ids->n);
    for (size_t r = 0; r < ids->n; ++r)
      ranges.push_back (ids->range[r]);
    free (ids);
    return ranges;
  }
};
}
