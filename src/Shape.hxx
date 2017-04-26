#pragma once

#include <utility>
#include "tinyhtm/tree.h"
#include "Spherical.hxx"
#include "Tree.hxx"
#include "Exception.hxx"

namespace tinyhtm
{
class Shape
{
public:
  virtual int64_t count (const Tree &) const
  {
    throw Exception ("Shape not valid");
  };
  virtual int64_t search (const Tree &, htm_callback) const
  {
    throw Exception ("Shape not valid");
  };
  virtual std::vector<htm_range> covering_ranges (const size_t &,
                                                  const size_t &) const
  {
    throw Exception ("Shape not valid");
  };
  virtual std::pair<Spherical, Spherical> bounding_box () const
  {
    throw Exception ("Shape not valid");
  };
  virtual ~Shape (){};
};
}
