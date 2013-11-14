#include "../Query.hxx"

namespace tinyhtm
{
  int64_t Query::count() const
  {
    int64_t count;
    enum htm_errcode ec;
    if(type==Type::circle)
      count=htm_tree_s2circle_count(&(tree.tree), &(center.v3), r, &ec);
    else if(type==Type::ellipse)
      count=htm_tree_s2ellipse_count(&(tree.tree), &(ellipse.ellipse), &ec);
    else if(type==Type::polygon)
      count=htm_tree_s2cpoly_count(&(tree.tree), poly, &ec);
    else
      throw Exception("Bad tinyhtm::Query::Type");
    return count;
  }
}
