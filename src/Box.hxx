#pragma once

#include <utility>

#include "Spherical.hxx"
#include "Cartesian.hxx"
#include "Shape.hxx"

namespace tinyhtm
{
class Box : public Shape
{
public:
  Spherical center, size;

  Box (const Spherical &Center, const Spherical &Size)
      : center (Center), size (Size)
  {
  }

  int64_t search (const Tree &tree, htm_callback callback) const override
  {
    struct htm_s2cpoly *poly = make_poly ();
    enum htm_errcode ec;
    int64_t count = htm_tree_s2cpoly (&(tree.tree), poly, &ec, callback);
    free (poly);
    if (ec != HTM_OK)
      {
        throw Exception ("Corrupted index file");
      }
    return count;
  }

  std::pair<Spherical, Spherical> bounding_box () const override
  {
    return std::make_pair (center, size);
  }

  struct htm_s2cpoly *make_poly () const
  {
    /// rotation must be zero for bounding box to be correct.
    double rotation = 0;
    Cartesian c (center);
    enum htm_errcode ec;
    struct htm_s2cpoly *poly
        = htm_s2cpoly_box (&(c.v3), size.lon (), size.lat (), rotation, &ec);
    if (ec != HTM_OK)
      {
        throw Exception ("Invalid box parameters: "
                         + std::to_string (center.lon ()) + " "
                         + std::to_string (center.lat ()) + " "
                         + std::to_string (size.lon ()) + " "
                         + std::to_string (size.lat ()));
      }
    return poly;
  }

  std::vector<Spherical> vertices () const
  {
    std::vector<Spherical> result;
    struct htm_s2cpoly *poly = make_poly ();
    try
      {
        for (size_t vv = 0; vv < poly->n; ++vv)
          {
            result.emplace_back (
                Cartesian (poly->ve[vv].x, poly->ve[vv].y, poly->ve[vv].z));
          }
      }
    catch (...)
      {
        free (poly);
        throw;
      }
    free (poly);
    return result;
  }
};
}
