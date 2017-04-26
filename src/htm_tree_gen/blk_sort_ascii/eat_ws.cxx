/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#include <cstddef>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <string>

char *eat_ws (char *s, char delim, const std::string &fname, size_t lineno)
{
  for (; isspace (*s) && *s != delim; ++s)
    {
    }
  if (*s == delim || *s == '\0')
    {
      std::stringstream ss;
      ss << "[" << fname << ":" << lineno << "] - invalid/truncated record";
      throw std::runtime_error (ss.str ());
    }
  return s;
}
