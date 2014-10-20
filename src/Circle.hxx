#pragma once

#include <utility>

#include "Spherical.hxx"
#include "Cartesian.hxx"

namespace tinyhtm
{
  class Circle: public Shape
  {
  public:
    Spherical center;
    double r;

    Circle(const Spherical &Center, const double &R): center(Center), r(R) {}

    int64_t search(const Tree &tree, htm_callback fn) const override
    {
      Cartesian c(center);
      enum htm_errcode ec;
      int64_t count=htm_tree_s2circle(&(tree.tree), &(c.v3), r, &ec, fn);
      if(ec!=HTM_OK)
        throw Exception("Corrupted index file");
      return count;
    }

    bool valid() const override
    {
      return true;
    }

    std::pair<Spherical,Spherical> bounding_box() const override
    {
      return std::make_pair(center,Spherical(2*r,2*r));
    }
  };
}
