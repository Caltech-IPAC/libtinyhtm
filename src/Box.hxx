#pragma once

#include <utility>

#include "Spherical.hxx"
#include "Cartesian.hxx"

namespace tinyhtm
{
  class Box: public Shape
  {
  public:
    Spherical center, size;
    double rotation;

    Box(const Spherical &Center, const Spherical &Size):
      center(Center), size(Size), rotation(0) {}

    int64_t search(const Tree &tree, htm_callback fn) const override
    {
      Cartesian c(center);
      enum htm_errcode ec;
      struct htm_s2cpoly *poly=htm_s2cpoly_box(&(c.v3), size.lon(),
                                               size.lat(), rotation, &ec);

      if(ec!=HTM_OK)
        throw Exception("Invalid box parameters: "
                        + std::to_string(center.lon()) + " "
                        + std::to_string(center.lat()) + " "
                        + std::to_string(size.lon()) + " "
                        + std::to_string(size.lat()));

      int64_t count=htm_tree_s2cpoly(&(tree.tree), poly, &ec, fn);
      free(poly);
      if(ec!=HTM_OK)
        throw Exception("Corrupted index file");
      return count;
    }

    std::pair<Spherical,Spherical> bounding_box() const override
    {
      return std::make_pair(center,size);
    }
  };
}