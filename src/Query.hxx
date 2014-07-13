#ifndef TINYHTM_QUERY_HXX
#define TINYHTM_QUERY_HXX

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
    struct htm_s2cpoly *poly;
    
    Query(const std::string &Data_file,
          const std::string &query_shape,
          const std::string &vertex_string);

    int64_t count() const;
    std::pair<int64_t,int64_t> range() const;
    int64_t search(std::function<bool (void *entry, int num_elements,
                                       hid_t *types, char **names)> fn) const;

    ~Query()
    {
      if(poly!=nullptr)
        free(poly);
    }
  };
}

#endif
