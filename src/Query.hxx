#ifndef TINYHTM_QUERY_HXX
#define TINYHTM_QUERY_HXX

#include <memory>
#include <vector>
#include <functional>
#include "Exception.hxx"
#include "Cartesian.hxx"
#include "Ellipse.hxx"
#include "Tree.hxx"

namespace tinyhtm
{
  class Query
  {
  public:
    Tree tree;
    std::unique_ptr<Shape> shape;
    
    /// Circle
    Query(const std::string &data_file, const Spherical &spherical,
          const double &R): tree(data_file),
                            shape(std::make_unique<Circle>(spherical,r)) {}

    /// Ellipse
    Query(const std::string &data_file,
          const Ellipse &e): tree(data_file),
                             shape(std::make_unique<Ellipse>(e)) {}

    /// Polygon
    Query(const std::string &data_file,
          const std::vector<Spherical> &vertices);

    /// Box
    Query(const std::string &data_file,
          const Spherical &ra_dec,
          const Spherical &width_height);

    /// Generic string
    Query(const std::string &data_file,
          const std::string &query_shape,
          const std::string &vertex_string);

    int64_t count() const
    {
      return shape->count();
    }
    int64_t search(htm_callback callback) const
    {
      return shape->search();
    }
  };
}

#endif
