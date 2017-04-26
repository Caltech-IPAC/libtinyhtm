#include "Spherical.hxx"
#include "Cartesian.hxx"

namespace tinyhtm
{
Spherical::Spherical (const Cartesian &c)
{
  enum htm_errcode ec = htm_v3_tosc (&sc, &(c.v3));
  if (ec != HTM_OK)
    {
      std::stringstream ss;
      ss << "Bad conversion from tinyhtm::Cartesian to tinyhtm::Spherical: "
         << c << htm_errmsg (ec);
      throw Exception (ss.str ());
    }
}
}
