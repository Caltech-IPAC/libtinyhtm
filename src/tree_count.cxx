/** \file
    \brief      Counting points in a region with HTM tree indexes.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */
#include <stdexcept>
#include <functional>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "tinyhtm/geometry.h"
#include "tinyhtm/tree.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \cond */

/* Should output be in JSON format or in IPAC SVC format? */
static int json = 0;
/* Should the count be estimated, or determined exactly? */
static int estimate = 0;
static int print = 0;

/* Performs string escaping for the JSON/IPAC SVC formats. */
static const char *esc (const char *s)
{
  static char buf[8192];
  char *e = buf;
  if (s == NULL)
    {
      return "null";
    }
  else
    {
      *e++ = '"';
      for (; *s != '\0' && e < buf + (sizeof(buf) - 2); ++s)
        {
          switch (*s)
            {
            case '"':
              e[0] = '\\';
              e[1] = '"';
              e += 2;
              break;
            case '\\':
              e[0] = '\\';
              e[1] = '\\';
              e += 2;
              break;
            case '\b':
              e[0] = '\\';
              e[1] = 'b';
              e += 2;
              break;
            case '\f':
              e[0] = '\\';
              e[1] = 'f';
              e += 2;
              break;
            case '\n':
              e[0] = '\\';
              e[1] = 'n';
              e += 2;
              break;
            case '\r':
              e[0] = '\\';
              e[1] = 'r';
              e += 2;
              break;
            case '\t':
              e[0] = '\\';
              e[1] = 't';
              e += 2;
              break;
            default:
              if (*s > 0x1f && *s < 0x7f)
                {
                  *e++ = *s;
                }
              break;
            }
        }
      if (*s != '\0' && e == buf + (sizeof(buf) - 2))
        {
          strcpy (buf + (sizeof(buf) - 6), " ...\"");
        }
      else
        {
          e[0] = '"';
          e[1] = '\0';
        }
    }
  return buf;
}

static void err (const char *fmt, ...)
{
  char buf[4095];
  int len;

  va_list ap;
  va_start (ap, fmt);
  len = vsnprintf (buf, sizeof(buf), fmt, ap);
  va_end (ap);
  if (len >= (int)sizeof(buf))
    {
      strcpy (buf + (sizeof(buf) - 5), " ...");
    }
  if (json)
    {
      printf ("{\"stat\":\"ERROR\", \"msg\":%s}\n", esc (buf));
    }
  else
    {
      printf ("[struct stat=\"ERROR\", msg=%s]\n", esc (buf));
    }
  fflush (stdout);
  exit (EXIT_FAILURE);
}

static double get_double (const char *s)
{
  char *endptr;
  double d = strtod (s, &endptr);
  if (errno != 0 || endptr == s)
    {
      err ("failed to convert argument `%s' to a double", s);
    }
  return d;
}

static void print_count (int64_t count)
{
  if (json)
    {
      printf ("{\"stat\":\"OK\", \"count\":%lld}\n", (long long)count);
    }
  else
    {
      printf ("[struct stat=\"OK\", count=\"%lld\"]\n", (long long)count);
    }
}

static void print_range (const struct htm_range *range)
{
  if (json)
    {
      printf ("{\"stat\":\"OK\", \"min\":%lld, \"max\":%lld}\n",
              (long long)range->min, (long long)range->max);
    }
  else
    {
      printf ("[struct stat=\"OK\", min=\"%lld\", max=\"%lld\"]\n",
              (long long)range->min, (long long)range->max);
    }
}

namespace
{
class Print_Entry
{
public:
  const std::vector<H5::DataType> &types;
  const std::vector<std::string> &names;
  bool print_header = true;
  Print_Entry (const std::vector<H5::DataType> &Types,
               const std::vector<std::string> &Names)
      : types (Types), names (Names)
  {
  }
  bool print (const char *entry)
  {
    if (print_header)
      {
        for (auto &name : names)
          printf ("%s\t", name.c_str ());
        printf ("\n");
        print_header = false;
      }
    size_t offset (0);
    for (auto &type : types)
      {
        if (type == H5::PredType::NATIVE_INT)
          {
            printf (" %ld", *((int64_t *)(entry + offset)));
            offset += sizeof(int64_t);
          }
        else if (type == H5::PredType::NATIVE_FLOAT)
          {
            printf (" %lf", *((float *)(entry + offset)));
            offset += sizeof(float);
          }
        else if (type == H5::PredType::NATIVE_DOUBLE)
          {
            printf (" %lf", *((double *)(entry + offset)));
            offset += sizeof(double);
          }
        else
          {
            throw std::runtime_error ("Unknown DataType: "
                                      + type.fromClass ());
          }
      }
    printf ("\n");
    return true;
  }
};
}

static void circle_count (const char *const datafile, char **argv)
{
  struct htm_tree tree;
  struct htm_sc sc;
  struct htm_v3 cen;
  double r;
  enum htm_errcode ec;

  ec = htm_sc_init (&sc, get_double (argv[0]), get_double (argv[1]));
  if (ec != HTM_OK)
    {
      err ("Invalid circle center coordinates: %s", htm_errmsg (ec));
    }
  ec = htm_sc_tov3 (&cen, &sc);
  if (ec != HTM_OK)
    {
      err ("Failed to convert spherical coordinates to a unit vector: %s",
           htm_errmsg (ec));
    }
  r = get_double (argv[2]);
  ec = htm_tree_init (&tree, datafile);
  if (ec != HTM_OK)
    {
      err ("Failed to load tree and/or data file: %s", htm_errmsg (ec));
    }
  if (estimate != 0)
    {
      struct htm_range range = htm_tree_s2circle_range (&tree, &cen, r, &ec);
      htm_tree_destroy (&tree);
      if (ec != HTM_OK)
        {
          err ("Failed to estimate points in circle: %s", htm_errmsg (ec));
        }
      print_range (&range);
    }
  else
    {
      int64_t count;
      if (print == 0)
        {
          count = htm_tree_s2circle (&tree, &cen, r, &ec, htm_callback ());
        }
      else
        {
          Print_Entry print_entry (tree.element_types, tree.element_names);
          count = htm_tree_s2circle (&tree, &cen, r, &ec,
                                     std::bind (&Print_Entry::print,
                                                &print_entry,
                                                std::placeholders::_1));
        }
      htm_tree_destroy (&tree);
      if (ec != HTM_OK)
        {
          err ("Failed to count points in circle: %s", htm_errmsg (ec));
        }
      print_count (count);
    }
}

static void ellipse_count (const char *const datafile, char **argv)
{
  struct htm_tree tree;
  struct htm_s2ellipse ellipse;
  struct htm_sc sc;
  struct htm_v3 cen;
  double a, b, angle;
  enum htm_errcode ec;

  ec = htm_sc_init (&sc, get_double (argv[0]), get_double (argv[1]));
  if (ec != HTM_OK)
    {
      err ("Invalid ellipse center coordinates: %s", htm_errmsg (ec));
    }
  ec = htm_sc_tov3 (&cen, &sc);
  if (ec != HTM_OK)
    {
      err ("Failed to convert spherical coordinates to a unit vector: %s",
           htm_errmsg (ec));
    }
  a = get_double (argv[2]);
  b = get_double (argv[3]);
  angle = get_double (argv[4]);
  ec = htm_s2ellipse_init2 (&ellipse, &cen, a, b, angle);
  if (ec != HTM_OK)
    {
      err ("Invalid ellipse parameters: %s", htm_errmsg (ec));
    }
  ec = htm_tree_init (&tree, datafile);
  if (ec != HTM_OK)
    {
      err ("Failed to load tree and/or data file: %s", htm_errmsg (ec));
    }
  if (estimate != 0)
    {
      struct htm_range range = htm_tree_s2ellipse_range (&tree, &ellipse, &ec);
      htm_tree_destroy (&tree);
      if (ec != HTM_OK)
        {
          err ("Failed to estimate points in ellipse: %s", htm_errmsg (ec));
        }
      print_range (&range);
    }
  else
    {
      int64_t count;
      if (print == 0)
        {
          count = htm_tree_s2ellipse (&tree, &ellipse, &ec, htm_callback ());
        }
      else
        {
          Print_Entry print_entry (tree.element_types, tree.element_names);
          count = htm_tree_s2ellipse (&tree, &ellipse, &ec,
                                      std::bind (&Print_Entry::print,
                                                 &print_entry,
                                                 std::placeholders::_1));
        }
      htm_tree_destroy (&tree);
      if (ec != HTM_OK)
        {
          err ("Failed to count points in ellipse: %s", htm_errmsg (ec));
        }
      print_count (count);
    }
}

static void hull_count (const char *const datafile, const int argc,
                        char **argv)
{
  struct htm_tree tree;
  struct htm_s2cpoly *poly;
  struct htm_sc sc;
  struct htm_v3 *verts;
  int i;
  enum htm_errcode ec = HTM_OK;

  verts = (struct htm_v3 *)malloc (sizeof(struct htm_v3) * (size_t)argc / 2);
  if (verts == NULL)
    {
      err (htm_errmsg (HTM_ENOMEM));
    }
  for (i = 0; i < argc / 2; ++i)
    {
      ec = htm_sc_init (&sc, get_double (argv[2 * i]),
                        get_double (argv[2 * i + 1]));
      if (ec != HTM_OK)
        {
          free (verts);
          err ("Invalid vertex coordinates: %s", htm_errmsg (ec));
        }
      ec = htm_sc_tov3 (&verts[i], &sc);
      if (ec != HTM_OK)
        {
          free (verts);
          err ("Failed to convert spherical coordinates to a unit vector: %s",
               htm_errmsg (ec));
        }
    }
  poly = htm_s2cpoly_hull (verts, (size_t)i, &ec);
  if (poly == NULL || ec != HTM_OK)
    {
      free (verts);
      err ("Failed to compute convex hull: %s", htm_errmsg (ec));
    }
  free (verts);
  ec = htm_tree_init (&tree, datafile);
  if (ec != HTM_OK)
    {
      free (poly);
      err ("Failed to load tree and/or data file: %s", htm_errmsg (ec));
    }
  if (estimate != 0)
    {
      struct htm_range range = htm_tree_s2cpoly_range (&tree, poly, &ec);
      htm_tree_destroy (&tree);
      free (poly);
      if (ec != HTM_OK)
        {
          err ("Failed to estimate points in hull: %s", htm_errmsg (ec));
        }
      print_range (&range);
    }
  else
    {
      int64_t count;
      if (print == 0)
        {
          count = htm_tree_s2cpoly (&tree, poly, &ec, htm_callback ());
        }
      else
        {
          Print_Entry print_entry (tree.element_types, tree.element_names);
          count = htm_tree_s2cpoly (
              &tree, poly, &ec, std::bind (&Print_Entry::print, &print_entry,
                                           std::placeholders::_1));
        }
      htm_tree_destroy (&tree);
      free (poly);
      if (ec != HTM_OK)
        {
          err ("Failed to count points in hull: %s", htm_errmsg (ec));
        }
      print_count (count);
    }
}

static void test_tree (const char *const datafile, char **argv)
{
  struct htm_tree tree;
  double r;
  unsigned long long i;
  enum htm_errcode ec;

  r = get_double (argv[0]);
  ec = htm_tree_init (&tree, datafile);
  if (ec != HTM_OK)
    {
      err ("Failed to load tree and/or data file: %s", htm_errmsg (ec));
    }
  for (i = 0; i < tree.count; ++i)
    {
      int64_t c = htm_tree_s2circle_count (
          &tree, (struct htm_v3 *)(static_cast<char *>(tree.entries)
                                   + i * tree.entry_size),
          r, &ec);
      if (c != 1)
        {
          err ("Circle of radius %g around %llu:(%.18g, %.18g, %.18g) "
               "contains %lld points (expecting 1).",
               r, i, ((struct htm_v3 *)(static_cast<char *>(tree.entries)
                                        + i * tree.entry_size))->x,
               ((struct htm_v3 *)(static_cast<char *>(tree.entries)
                                  + i * tree.entry_size))->y,
               ((struct htm_v3 *)(static_cast<char *>(tree.entries)
                                  + i * tree.entry_size))->z,
               (long long)c);
        }
    }
  htm_tree_destroy (&tree);
}

static void usage (const char *prog)
{
  printf (
      "%1$s [options] <file> circle <ra> <dec> <radius>\n"
      "\n"
      "%1$s [options] <file> ellipse <ra> <dec> <axis1> <axis2> <angle>\n"
      "\n"
      "%1$s [options] <file> hull <ra_1> <dec_1> <ra_2> <dec_2> ...<ra_n> "
      "<dec_n>\n"
      "\n"
      "%1$s [options] <file> test <radius>\n"
      "\n"
      "    This utility counts the number of points from <file> that\n"
      "lie in the given spherical region. Note one particularly important\n"
      "option: --tree (-t). This option specifies a tree index file that\n"
      "can be used to greatly speed up point counting. Three different\n"
      "types of spherical region are supported:\n"
      "\n"
      " Circle\n"
      "    A circle is specified by the spherical coordinates of its\n"
      "    center and its radius. All parameters are assumed to be in\n"
      "    degrees.\n"
      "\n"
      " Ellipse\n"
      "    An ellipse is specified by the spherical coordinates of its\n"
      "    center, two axis angles, and a position angle giving the\n"
      "    the orientation (east of north) of the first axis. All\n"
      "    parameters are assumed to be in degrees.\n"
      "\n"
      " Convex Hull\n"
      "    A convex hull is given by a hemispherical set of at least\n"
      "    3 points. Points are specified by their spherical coordinates;\n"
      "    coordinate values are assumed to be in degrees.\n"
      "\n"
      "    Finally, the utility supports a tree file integrity test.\n"
      "This mode constructs circles of the given radius around every\n"
      "point in <file>, and checks that the point count for that circle\n"
      "is exactly one. Note that this is useful only when points are\n"
      "unique and one knows their minimum separation.\n"
      "\n"
      "== Options ====\n"
      "\n"
      "--help     | -h              :  Prints usage information.\n"
      "--estimate | -e              :  Estimate count instead of determining\n"
      "                                the exact value. Has no effect unless\n"
      "                                used in conjunction with --tree.\n"
      "--print    | -p              :  Print values of matching entries in\n"
      "                                addition to the total count\n"
      "--json     | -j              :  Print results in JSON format.\n"
      "                                the input points.\n",
      prog);
}

int main (int argc, char **argv)
{
  char *datafile = NULL;

  opterr = 0;
  while (1)
    {
      static struct option long_options[]
          = { { "help", no_argument, 0, 'h' },
              { "json", no_argument, 0, 'j' },
              { "estimate", no_argument, 0, 'e' },
              { "print", no_argument, 0, 'p' },
              { 0, 0, 0, 0 } };
      int option_index = 0;
      int c = getopt_long (argc, argv, "+ephjt:", long_options, &option_index);
      if (c == -1)
        {
          break; /* no more options */
        }
      switch (c)
        {
        case 'e':
          estimate = 1;
          break;
        case 'p':
          print = 1;
          break;
        case 'h':
          usage (argv[0]);
          return EXIT_SUCCESS;
        case 'j':
          json = 1;
          break;
        case '?':
          err ("Unknown option. Pass --help for usage instructions");
          break;
        default:
          abort ();
        }
    }
  if (argc - optind < 2)
    {
      err ("Missing arguments. Pass --help for usage instructions.");
    }
  datafile = argv[optind];
  if (strcmp (argv[optind + 1], "circle") == 0)
    {
      optind += 2;
      if (argc - optind != 3)
        {
          err ("Missing arguments. Pass --help for usage instructions.");
        }
      circle_count (datafile, argv + optind);
    }
  else if (strcmp (argv[optind + 1], "ellipse") == 0)
    {
      optind += 2;
      if (argc - optind != 5)
        {
          err ("Missing arguments. Pass --help for usage instructions.");
        }
      ellipse_count (datafile, argv + optind);
    }
  else if (strcmp (argv[optind + 1], "hull") == 0)
    {
      optind += 2;
      if (argc - optind < 6 || (argc - optind) % 2 != 0)
        {
          err ("Missing arguments. Pass --help for usage instructions.");
        }
      hull_count (datafile, argc - optind, argv + optind);
    }
  else if (strcmp (argv[optind + 1], "test") == 0)
    {
      optind += 2;
      if (argc - optind != 1)
        {
          err ("Missing arguments. Pass --help for usage instructions.");
        }
      test_tree (datafile, argv + optind);
      if (json)
        {
          printf ("{\"stat\":\"OK\"}\n");
        }
      else
        {
          printf ("[struct stat=\"OK\"]\n");
        }
      fflush (stdout);
      return EXIT_SUCCESS;
    }
  else
    {
      err ("Invalid region type '%s' - expecting 'circle', "
           "'ellipse', or 'hull'",
           argv[optind + 1]);
    }
  fflush (stdout);
  return EXIT_SUCCESS;
}

/** \endcond */

#ifdef __cplusplus
}
#endif
