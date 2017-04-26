#ifndef TINYHTM_CARTESIAN_HXX
#define TINYHTM_CARTESIAN_HXX

#include <sstream>
#include "tinyhtm.h"
#include "Exception.hxx"
#include <iostream>

namespace tinyhtm
{
class Spherical;
class Cartesian
{
public:
  htm_v3 v3;
  Cartesian (const double &x, const double &y, const double &z)
  {
    enum htm_errcode ec = htm_v3_init (&v3, x, y, z);
    if (ec != HTM_OK)
      {
        std::stringstream ss;
        ss << "Bad coordinates for initializing tinyhtm::Cartesian: " << x
           << " " << y << " " << z << " " << htm_errmsg (ec);
        throw Exception (ss.str ());
      }
  }
  Cartesian () = default;
  Cartesian (const Spherical &s);
  double x () const { return v3.x; }
  double y () const { return v3.y; }
  double z () const { return v3.z; }
};

inline std::ostream &operator<<(std::ostream &os, const Cartesian &s)
{
  return (os << "(" << s.v3.x << " " << s.v3.y << " " << s.v3.z << " "
             << ")");
}
}
#endif
