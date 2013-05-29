#include <boost/lexical_cast.hpp>
#include "Query.hxx"
#include "Spherical.hxx"

namespace tinyhtm
{
  Query::Query(const std::string &tree_file, const std::string &data_file,
               char *args[], const int &n): tree(tree_file,data_file),
                                            poly(nullptr)
  {
    if(args[0]==std::string("circle"))
      {
        type=tinyhtm::Query::Type::circle;
        if(n!=4)
          {
            std::stringstream ss;
            ss << "Wrong number of arguments for circle.  Need 4 but have "
               << n;
            throw Exception(ss.str());
          }
        center=Cartesian(Spherical(boost::lexical_cast<double>(args[1]),
                                   boost::lexical_cast<double>(args[2])));
        r=boost::lexical_cast<double>(args[3]);
      }
    else if(args[0]==std::string("ellipse"))
      {
        type=tinyhtm::Query::Type::ellipse;
        if(n!=6)
          {
            std::stringstream ss;
            ss << "Wrong number of arguments for ellipse.  Need 6 but have "
               << n;
            throw Exception(ss.str());
          }
        ellipse=Ellipse(Spherical(boost::lexical_cast<double>(args[1]),
                                  boost::lexical_cast<double>(args[2])),
                        boost::lexical_cast<double>(args[3]),
                        boost::lexical_cast<double>(args[4]),
                        boost::lexical_cast<double>(args[5]));
      }
    else if(args[0]==std::string("polygon"))
      {
        type=tinyhtm::Query::Type::polygon;
        if(n<7)
          {
            std::stringstream ss;
            ss << "Not enough arguments for polygon.  "
              "Need at least 7 but only have "
               << n;
            throw Exception(ss.str());
          }
        if(n%2!=1)
          {
            std::stringstream ss;
            ss << "Need an even number of arguments for polygon, "
               << "but was given " << n-1;
            throw Exception(ss.str());
          }
        for(int j=1;j<n;j+=2)
          {
            Cartesian c(Spherical(boost::lexical_cast<double>(args[j]),
                                  boost::lexical_cast<double>(args[j+1])));
            verts.push_back(c.v3);
          }
        enum htm_errcode ec;
        poly=htm_s2cpoly_hull(verts.data(), verts.size(), &ec);
        if(poly==NULL || ec!=HTM_OK)
          throw Exception("Can not allocate polygon");
      }
    else
      throw Exception(std::string("Bad query type: ") + args[0]);
  }
}
