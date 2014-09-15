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
    enum class Type {circle,ellipse,polygon};
    
    Tree tree;
    Type type;
    Cartesian center;
    double r;
    Ellipse ellipse;
    std::vector<htm_v3> verts;
    std::unique_ptr<htm_s2cpoly> poly;
    
    /// Circle
    Query(const std::string &data_file, const Spherical &spherical,
          const double &R): tree(data_file), type(tinyhtm::Query::Type::circle),
                            center(spherical), r(R) {}

    /// Ellipse
    Query(const std::string &data_file,
          const Ellipse &e): tree(data_file), 
                             type(tinyhtm::Query::Type::ellipse), ellipse(e) {}

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

    int64_t count() const;
    std::pair<int64_t,int64_t> range() const;
    int64_t search(htm_callback fn) const;
  };
}

#endif
