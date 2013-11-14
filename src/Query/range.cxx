#include "../Query.hxx"

namespace tinyhtm
{
  std::pair<int64_t,int64_t> Query::range() const
  {
    struct htm_range range;
    enum htm_errcode ec;
    if(type==Type::circle)
      range=htm_tree_s2circle_range(&(tree.tree), &(center.v3), r, &ec);
    else if(type==Type::ellipse)
      range=htm_tree_s2ellipse_range(&(tree.tree), &(ellipse.ellipse), &ec);
    else if(type==Type::polygon)
      range=htm_tree_s2cpoly_range(&(tree.tree), poly, &ec);
    else
      throw Exception("Bad tinyhtm::Query::Type");
    return std::make_pair(range.min,range.max);
  }
}
