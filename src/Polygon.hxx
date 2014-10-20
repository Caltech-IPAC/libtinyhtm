#pragma once

#include <utility>
#include <limits>

#include "Spherical.hxx"
#include "Cartesian.hxx"

namespace tinyhtm
{
  class Polygon: public Shape
  {
  public:
    std::vector<Spherical> vertices;

    Polygon(const std::vector<Spherical> &Vertices):
      vertices(Vertices) {}

    int64_t search(const Tree &tree, htm_callback fn) const override
    {
      enum htm_errcode ec;
      
      std::vector<struct htm_v3> poly_vertices;
      for(auto &v: vertices)
        poly_vertices.push_back(Cartesian(v).v3);
      struct htm_s2cpoly *poly=htm_s2cpoly_init(poly_vertices.data(),
                                                poly_vertices.size(), &ec);

      int64_t count=htm_tree_s2cpoly(&(tree.tree), poly, &ec, fn);
      free(poly);
      if(ec!=HTM_OK)
        throw Exception("Corrupted index file");
      return count;
    }

    std::pair<Spherical,Spherical> bounding_box() const override
    {
      Spherical min(std::numeric_limits<double>::max(),
                    std::numeric_limits<double>::max()),
        max(std::numeric_limits<double>::lowest(),
            std::numeric_limits<double>::lowest());
      for(auto &v: vertices)
        {
          min.lon()=std::min(min.lon(),v.lon());
          min.lat()=std::min(min.lat(),v.lat());
          max.lon()=std::max(max.lon(),v.lon());
          max.lat()=std::max(max.lat(),v.lat());
        }

      Spherical center((min.lon()+max.lon())/2,(min.lat()+max.lat())/2),
        size(max.lon()-min.lon(),max.lat()-min.lat());
      return std::make_pair(center,size);
    }
  };
}
