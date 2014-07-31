#include <boost/lexical_cast.hpp>
#include "../Query.hxx"
#include "../Spherical.hxx"

/// Box
tinyhtm::Query::Query(const std::string &data_file,
                      const Spherical &ra_dec,
                      const Spherical &width_height)
  : tree(data_file), type(tinyhtm::Query::Type::polygon)
{
  Cartesian center(ra_dec);
  enum htm_errcode ec;
  poly=std::unique_ptr<htm_s2cpoly>(htm_s2cpoly_box(&(center.v3),
                                                    width_height.lon(),
                                                    width_height.lat(), 0, &ec));
  if(poly==nullptr || ec!=HTM_OK)
    throw Exception("Can not allocate box");
}

/// Polygon
tinyhtm::Query::Query(const std::string &data_file,
                      const std::vector<Spherical> &vertices)
  : tree(data_file), type(tinyhtm::Query::Type::polygon)
{
  for(auto &v: vertices)
    verts.push_back(Cartesian(v).v3);

  enum htm_errcode ec;
  poly=std::unique_ptr<htm_s2cpoly>(htm_s2cpoly_hull(verts.data(),
                                                     verts.size(), &ec));
  if(poly==nullptr || ec!=HTM_OK)
    throw Exception("Can not allocate polygon");
}

tinyhtm::Query::Query(const std::string &data_file,
                      const std::string &query_shape,
                      const std::string &vertex_string): tree(data_file)
{
  std::stringstream ss(vertex_string);
  std::vector<double> vertices;
  double x;
  ss >> x;
  while(ss)
    {
      vertices.push_back(x);
      ss >> x;
    }

  if(query_shape=="circle")
    {
      type=tinyhtm::Query::Type::circle;
      if(vertices.size()!=3)
        {
          std::stringstream ss;
          ss << "Wrong number of arguments for circle.  Need 3 but have "
             << vertices.size()
             << ": " << vertex_string << "\n"
             << vertices[0] << "\n"
             << vertices[1] << "\n"
             << vertices[2] << "\n"
             << vertices[3] << "\n";
          throw Exception(ss.str());
        }
      center=Cartesian(Spherical(vertices[0],vertices[1]));
      r=vertices[2];
    }
  else if(query_shape=="ellipse")
    {
      type=tinyhtm::Query::Type::ellipse;
      if(vertices.size()!=5)
        {
          std::stringstream ss;
          ss << "Wrong number of arguments for ellipse.  Need 5 but have "
             << vertices.size();
          throw Exception(ss.str());
        }
      ellipse=Ellipse(Spherical(vertices[0],vertices[1]),
                      vertices[2],vertices[3],vertices[4]);
    }
  else if(query_shape=="polygon")
    {
      type=tinyhtm::Query::Type::polygon;
      if(vertices.size()<6)
        {
          std::stringstream ss;
          ss << "Not enough arguments for polygon.  "
            "Need at least 6 but only have "
             << vertices.size();
          throw Exception(ss.str());
        }
      if(vertices.size()%2!=0)
        {
          std::stringstream ss;
          ss << "Need an even number of arguments for polygon, "
             << "but was given " << vertices.size();
          throw Exception(ss.str());
        }
      for(size_t j=0;j<vertices.size();j+=2)
        {
          Cartesian c(Spherical(vertices[j],vertices[j+1]));
          verts.push_back(c.v3);
        }
      enum htm_errcode ec;
      poly=std::unique_ptr<htm_s2cpoly>(htm_s2cpoly_hull(verts.data(),
                                                         verts.size(), &ec));
      if(poly==nullptr || ec!=HTM_OK)
        throw Exception("Can not allocate polygon");
    }
  else if(query_shape=="box")
    {
      type=tinyhtm::Query::Type::polygon;
      if(vertices.size()!=4)
        {
          std::stringstream ss;
          ss << "Wrong number of arguments for box.  Need 4 but have "
             << vertices.size()
             << ": " << vertex_string << "\n"
             << vertices[0] << "\n"
             << vertices[1] << "\n"
             << vertices[2] << "\n"
             << vertices[3] << "\n";
          throw Exception(ss.str());
        }
      htm_sc sc;
      htm_v3 cen;
      enum htm_errcode ec;
      ec=htm_sc_init(&sc,vertices[0],vertices[1]);
      ec = htm_sc_tov3(&cen, &sc);
      poly=std::unique_ptr<htm_s2cpoly>(htm_s2cpoly_box(&cen, vertices[2],
                                                        vertices[3], 0, &ec));
      if(poly==nullptr || ec!=HTM_OK)
        throw Exception("Can not allocate box");
      for(size_t j=0;j<poly->n;j++)
        verts.push_back(poly->ve[j]);
    }
  else
    throw Exception(std::string("Bad query shape: ") + query_shape);
}
