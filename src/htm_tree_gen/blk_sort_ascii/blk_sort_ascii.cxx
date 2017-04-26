/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
*/

/*  Converts ASCII input files to a block-sorted binary file.
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <sstream>
#include "../../sort_and_index/blk_writer.hxx"
#include "../../sort_and_index/mem_params.hxx"
#include "../../tree_entry.hxx"
#include "../../sort_and_index/now.hxx"

char *eat_delim (char *s, char delim, const std::string &fname, size_t lineno);
char *eat_ws (char *s, char delim, const std::string &fname, size_t lineno);

size_t blk_sort_ascii (const std::vector<std::string> &infiles,
                       const std::string &outfile, const char delim,
                       const mem_params *const mem)
{
  char line[16384];
  size_t nentries;
  double ttot;

  if (mem->sortsz % sizeof(struct tree_entry) != 0)
    {
      throw std::runtime_error ("Write block size is not a multiple of "
                                "sizeof(struct tree_entry)");
    }
  std::cout << "Creating block-sorted tree entry file " << outfile
            << " from ASCII file(s)\n";
  ttot = now ();
  nentries = 0;
  blk_writer<tree_entry> out (outfile, mem->sortsz);

  /* For each input file... */
  for (auto &infile : infiles)
    {
      FILE *f;
      size_t lineno;
      struct tree_entry entry;
      struct htm_v3 v;
      double t;

      std::cout << "\t- processing " << infile << " ... ";
      t = now ();

      /* open input file */
      f = fopen (infile.c_str (), "r");
      if (f == NULL)
        {
          throw std::runtime_error ("Failed to open file " + infile
                                    + " for reading");
        }
      for (lineno = 1; fgets (line, sizeof(line), f) != NULL; ++lineno)
        {
          char *s, *endptr;
          double lon, lat;
          size_t len = strlen (line);

          if (line[len - 1] != '\n')
            {
              if (feof (f) == 0)
                {
                  std::stringstream ss;
                  ss << "Line " << lineno << " of file " << infile
                     << " is too long (> " << sizeof(line) << "characters)";
                  throw std::runtime_error (ss.str ());
                }
            }
          s = eat_ws (line, delim, infile, lineno);
          entry.rowid = (int64_t)strtoll (s, &endptr, 0);
          if (endptr == s || endptr == NULL || errno != 0)
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno
                 << "] - failed to convert row_id to an integer";
              throw std::runtime_error (ss.str ());
            }
          s = eat_delim (endptr, delim, infile, lineno);
          s = eat_ws (s, delim, infile, lineno);
          lon = strtod (s, &endptr);
          if (endptr == s || endptr == NULL || errno != 0)
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno
                 << "] - failed to convert right ascension/longitude to a "
                    "double";
              throw std::runtime_error (ss.str ());
            }
          s = eat_delim (endptr, delim, infile, lineno);
          s = eat_ws (s, delim, infile, lineno);
          lat = strtod (s, &endptr);
          if (endptr == s || endptr == NULL || errno != 0)
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno
                 << "] - failed to convert declination/latitude to a double";
              throw std::runtime_error (ss.str ());
            }
          s = endptr;
          if (*s != delim && *s != '\0' && !isspace (*s))
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno << "] - invalid record";
              throw std::runtime_error (ss.str ());
            }
          /* compute and store tree_entry for line */
          if (htm_sc_init (&entry.sc, lon, lat) != HTM_OK)
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno
                 << "] - invalid spherical coordinates";
              throw std::runtime_error (ss.str ());
            }
          if (htm_sc_tov3 (&v, &entry.sc) != HTM_OK)
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno
                 << "] - failed to convert spherical coordinates "
                 << "to a unit vector";
              throw std::runtime_error (ss.str ());
            }
          entry.htmid = htm_v3_id (&v, 20);
          if (entry.htmid == 0)
            {
              std::stringstream ss;
              ss << "[" << infile << ":" << lineno
                 << "] - failed to compute HTM ID for spherical "
                 << "coordinates";
              throw std::runtime_error (ss.str ());
            }
          out.append (&entry);
        }
      if (ferror (f) != 0)
        {
          throw std::runtime_error ("failed to read file " + infile);
        }
      if (fclose (f) != 0)
        {
          throw std::runtime_error ("failed to close file " + infile);
        }
      nentries += lineno - 1;
      std::cout << lineno << " records in " << now () - t << " sec\n";
      /* advance to next input file */
    }

  std::cout << "\t" << now () - ttot << " sec for " << nentries
            << " records total\n\n";
  return nentries;
}
