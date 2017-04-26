/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

#include <stdio.h>

void usage (const char *prog)
{
  printf (
      "%s [options] <out_path> <in_file_1> [<in_file_2> ...]\n"
      "\n"
      "    This utility takes character-separated-value files\n"
      "containing spherical coordinates, and outputs an HTM tree index\n"
      "useful for fast point-in-spherical-region counting. The first\n"
      "column of the input files must correspond to unique row IDs\n"
      "(64-bit integers), the second to right ascension/longitude angles\n"
      "(in degrees), and the third to declination/latitude angles (also\n"
      "in degrees). Additional columns are allowed but ignored.\n"
      "\n"
      "    Typically, two output files named <out_path>.htm and\n"
      "<out_path>.dat are generated, though the first may optionally\n"
      "be omitted.\n"
      "\n"
      "    The first is a tree index, and the second contains points\n"
      "sorted by level 20 HTM ID. If the inputs contain no more than a\n"
      "configurable number of points, the tree index is omitted. When is\n"
      "this useful? For small data-sets, scanning the point file and\n"
      "testing each point for spherical-region membership can be faster\n"
      "than searching a tree. This is because tree searching has some\n"
      "overhead, and for sufficiently small data sets, a single page\n"
      "on disk will contain a significant fraction of the points. In\n"
      "this case, using a tree to reduce the number of points tested\n"
      "against a region fails to reduce IO compared to a scan.\n"
      "\n"
      "== Options ====\n"
      "\n"
      "--help        |-h        :  Prints usage information.\n"
      "--blk-size    |-b <int>  :  IO block size in KiB. The default is\n"
      "                            1024 KiB.\n"
      "--delim       |-d <char> :  The separator character to use when\n"
      "                            parsing input files. The default is '|'.\n"
      "--max-mem     |-m <int>  :  Approximate memory usage limit in MiB.\n"
      "                            The default is 512 MiB. Note that this\n"
      "                            applies only to various external sorts,\n"
      "                            and that tree generation can use up to\n"
      "                            about 512MiB of memory regardless.\n"
      "--tree-min    |-t <int>  :  If the total number of input points does\n"
      "                            not exceed the specified number, tree\n"
      "                            index generation is skipped. The default\n"
      "                            is 1024.\n"
      "--leaf-thresh |-l <int>  :  Minimum number of points in an internal\n"
      "                            tree node; defaults to 64.\n",
      prog);
}
