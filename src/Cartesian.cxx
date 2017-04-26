#include "Spherical.hxx"
#include "Cartesian.hxx"

namespace tinyhtm
{
Cartesian::Cartesian (const Spherical &s)
{
  enum htm_errcode ec = htm_sc_tov3 (&v3, &(s.sc));
  if (ec != HTM_OK)
    {
      std::stringstream ss;
      ss << "Bad conversion from tinyhtm::Cartesian to tinyhtm::Spherical: "
         << s << htm_errmsg (ec);
      throw Exception (ss.str ());
    }
}
}
