#ifndef TINYHTM_ELLIPSE_HXX
#define TINYHTM_ELLIPSE_HXX

namespace tinyhtm
{
  class Ellipse
  {
  public:
    struct htm_s2ellipse ellipse;
    Ellipse(const Spherical &s, const double &a, const double &b,
            const double &angle)
    {
      Cartesian c(s);
      enum htm_errcode ec = htm_s2ellipse_init2(&ellipse,&(c.v3),a,b,angle);
      if(ec!=HTM_OK)
        {
          std::stringstream ss;
          ss << "Bad coordinates for initializing tinyhtm::Ellipse: "
             << s << " "
             << a << " " << b << " " << angle << " "
             << htm_errmsg(ec);
          throw Exception(ss.str());
        }
    }
    Ellipse(){}
  };
}

#endif

