#pragma once

#include <utility>
#include <limits>

#include "Spherical.hxx"
#include "Cartesian.hxx"
#include "Shape.hxx"

namespace tinyhtm
{
class Polygon : public Shape
{
public:
  std::vector<Spherical> vertices;

  Polygon (const std::vector<Spherical> &Vertices) : vertices (Vertices) {}

  int64_t search (const Tree &tree, htm_callback callback) const override
  {
    enum htm_errcode ec;

    std::vector<struct htm_v3> poly_vertices;
    for (auto &v : vertices)
      poly_vertices.push_back (Cartesian (v).v3);
    struct htm_s2cpoly *poly
        = htm_s2cpoly_init (poly_vertices.data (), poly_vertices.size (), &ec);

    int64_t count = htm_tree_s2cpoly (&(tree.tree), poly, &ec, callback);
    free (poly);
    if (ec != HTM_OK)
      throw Exception ("Corrupted index file");
    return count;
  }

  std::pair<Spherical, Spherical> bounding_box () const override
  {
    Spherical min (std::numeric_limits<double>::max (),
                   std::numeric_limits<double>::max ()),
        max (std::numeric_limits<double>::lowest (),
             std::numeric_limits<double>::lowest ());
    for (auto &v : vertices)
      {
        // FIXME: This looks broken near the poles, where min(lat)
        // can be the same as max(lat).
        min.lon () = std::min (min.lon (), v.lon ());
        min.lat () = std::min (min.lat (), v.lat ());
        max.lon () = std::max (max.lon (), v.lon ());
        max.lat () = std::max (max.lat (), v.lat ());
      }

    Spherical center ((min.lon () + max.lon ()) / 2,
                      (min.lat () + max.lat ()) / 2),
        size (max.lon () - min.lon (), max.lat () - min.lat ());
    return std::make_pair (center, size);
  }

  std::vector<htm_range>
  covering_ranges (const size_t &level,
                   const size_t &max_ranges) const override
  {
    std::vector<htm_v3> xyz_vertices;
    for (auto &v : vertices)
      xyz_vertices.emplace_back (Cartesian (v).v3);
    htm_s2cpoly *poly_cartesian = nullptr;
    enum htm_errcode ec;
    poly_cartesian
        = htm_s2cpoly_hull (xyz_vertices.data (), xyz_vertices.size (), &ec);
    if (ec != HTM_OK)
      {
        if (poly_cartesian != nullptr)
          free (poly_cartesian);
        throw Exception (
            std::string ("Failed to convert a polygon from spherical "
                         "coordinates to unit vectors: ") + htm_errmsg (ec));
      }

    struct htm_ids *ids = nullptr;
    ids = htm_s2cpoly_ids (ids, poly_cartesian, level, max_ranges, &ec);
    free (poly_cartesian);
    if (ec != HTM_OK)
      {
        if (ids != nullptr)
          free (ids);
        throw Exception (
            std::string ("Failed to find HTM triangles overlapping polygon: ")
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
