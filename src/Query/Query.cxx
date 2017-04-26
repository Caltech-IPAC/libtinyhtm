#include "../Query.hxx"

/// Generic string
tinyhtm::Query::Query (const std::string &data_file,
                       const std::string &query_shape,
                       const std::string &vertex_string)
    : tree (data_file)
{
  std::stringstream ss (vertex_string);
  std::vector<double> numbers;
  double x;
  ss >> x;
  while (ss)
    {
      numbers.push_back (x);
      ss >> x;
    }

  if (query_shape == "circle")
    {
      if (numbers.size () != 3)
        {
          throw Exception ("Wrong number of arguments for circle.  "
                           "Need 3 but have "
                           + std::to_string (numbers.size ()));
        }
      shape = std::make_unique<Circle>(Spherical (numbers[0], numbers[1]),
                                       numbers[2]);
    }
  else if (query_shape == "ellipse")
    {
      if (numbers.size () != 5)
        {
          throw Exception ("Wrong number of arguments for ellipse.  "
                           "Need 5 but have "
                           + std::to_string (numbers.size ()));
        }
      shape = std::make_unique<Ellipse>(Spherical (numbers[0], numbers[1]),
                                        numbers[2], numbers[3], numbers[4]);
    }
  else if (query_shape == "polygon")
    {
      if (numbers.size () < 6)
        {
          throw Exception ("Not enough arguments for polygon.  "
                           "Need at least 6 but only have "
                           + std::to_string (numbers.size ()));
        }
      if (numbers.size () % 2 != 0)
        {
          throw Exception ("Need an even number of arguments for polygon, "
                           "but was given "
                           + std::to_string (numbers.size ()));
        }
      std::vector<Spherical> vertices;
      for (size_t j = 0; j < numbers.size (); j += 2)
        {
          vertices.emplace_back (numbers[j], numbers[j + 1]);
        }
      shape = std::make_unique<Polygon>(vertices);
    }
  else if (query_shape == "box")
    {
      if (numbers.size () != 4)
        {
          throw Exception ("Wrong number of arguments for box.  "
                           "Need 4 but have "
                           + std::to_string (numbers.size ()));
        }
      shape = std::make_unique<Box>(Spherical (numbers[0], numbers[1]),
                                    Spherical (numbers[2], numbers[3]));
    }
  else
    {
      throw Exception (std::string ("Bad query shape: ") + query_shape);
    }
}
