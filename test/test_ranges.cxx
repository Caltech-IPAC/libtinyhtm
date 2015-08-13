#include <array>

#include "Circle.hxx"
#include "Ellipse.hxx"
#include "Polygon.hxx"

bool check_ranges (const std::vector<htm_range> &ranges,
                   const std::array<int64_t,4> &min_range,
                   const std::array<int64_t,4> &max_range,
                   const std::string &shape)
{
  bool result=true;
  if (ranges.size () != min_range.size ())
    result=false;
  if (result)
    for (size_t i=0; i<ranges.size (); ++i)
      result = result && ranges[i].min==min_range[i]
        && ranges[i].max==max_range[i];
  if (result)
    {
      std::cout << "PASS: ";
    }
  else
    {
      std::cout << "FAIL: ";
    }
  std::cout << shape << " covering ranges\n";
  return result;
}

int main()
{
  const size_t htm_level(20), max_ranges(4);
  tinyhtm::Circle circle(tinyhtm::Spherical(27.1828,31.415),1.414);
  auto ranges(circle.covering_ranges(htm_level,max_ranges));
  std::array<int64_t,4> circle_min_range{{17335561748480,17336367054848,
        17349520392192,17350057263104}},
    circle_max_range{{17336098619391,17336635490303,
          17349788827647,17350862569471}};
  bool circle_result=check_ranges(ranges,circle_min_range,circle_max_range,"Circle");
  
  tinyhtm::Ellipse ellipse(tinyhtm::Spherical(27.1828,31.415),14.14,1.618,42);
  ranges=ellipse.covering_ranges(htm_level,max_ranges);
  std::array<int64_t,4> ellipse_min_range{{17317308137472,17368847745024,
      17420387352576,17557826306048}},
  ellipse_max_range{{17351667875839,17386027614207,
        17437567221759,17575006175231}};
  bool ellipse_result=check_ranges(ranges,ellipse_min_range,ellipse_max_range,"Ellipse");
  
  tinyhtm::Polygon polygon({tinyhtm::Spherical(27.1828,31.415),
        tinyhtm::Spherical(29.1828,33.415),
        tinyhtm::Spherical(23.1828,35.415)});
  ranges=polygon.covering_ranges(20,4);
  std::array<int64_t,4> polygon_min_range{{17340930457600,17341467328512,
        17347641344000,17349520392192}},
    polygon_max_range{{17341198893055,17342004199423,
          17348446650367,17351667875839}};
  bool polygon_result=check_ranges(ranges,polygon_min_range,polygon_max_range,"Polygon");

  bool result = circle_result && ellipse_result && polygon_result;
  return result ? 0 : 1;
}
