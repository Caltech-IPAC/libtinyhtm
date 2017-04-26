#ifndef TINYHTM_EXCEPTION_HXX
#define TINYHTM_EXCEPTION_HXX

#include <stdexcept>

namespace tinyhtm
{
class Exception : public std::runtime_error
{
public:
  Exception (const std::string &s) : std::runtime_error (s) {}
};
}

#endif
