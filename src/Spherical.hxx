#ifndef TINYHTM_SPHERICAL_HXX
#define TINYHTM_SPHERICAL_HXX

#include "tinyhtm.h"
#include "Exception.hxx"
#include <sstream>
#include <iostream>

namespace tinyhtm
{
class Cartesian;
class Spherical
{
public:
  htm_sc sc;
  Spherical (const double &lon, const double &lat)
  {
    enum htm_errcode ec = htm_sc_init (&sc, lon, lat);
    if (ec != HTM_OK)
      {
        std::stringstream ss;
        ss << "Bad coordinates when initializing tinyhtm::Spherical: " << lon
           << " " << lat << " " << htm_errmsg (ec);
        throw Exception (ss.str ());
      }
  }

  Spherical (const Cartesian &c);
  Spherical (const Spherical &s) : Spherical (s.lon (), s.lat ()) {}
  Spherical () {}

  double lon () const { return sc.lon; }
  double &lon () { return sc.lon; }
  double lat () const { return sc.lat; }
  double &lat () { return sc.lat; }
};

inline std::ostream &operator<<(std::ostream &os, const Spherical &s)
{
  return (os << "(" << s.sc.lon << " " << s.sc.lat << ")");
}
}
#endif
