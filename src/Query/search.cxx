#include "../Query.hxx"

namespace tinyhtm
{
  int64_t Query::search(htm_callback fn) const
  {
    int64_t count;
    enum htm_errcode ec;
    if(type==Type::circle)
      {
        count=htm_tree_s2circle(&(tree.tree), &(center.v3), r, &ec, fn);
      }
    else if(type==Type::ellipse)
      {
        count=htm_tree_s2ellipse(&(tree.tree), &(ellipse.ellipse), &ec, fn);
      }
    else if(type==Type::polygon)
      {
        count=htm_tree_s2cpoly(&(tree.tree), poly.get(), &ec, fn);
      }
    else
      throw Exception("Bad tinyhtm::Query::Type");

    return count;
  }
}