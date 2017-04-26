/** \file
    \brief      HTM tree generation.

    \authors    Serge Monkewitz
    \copyright  IPAC/Caltech
  */

/** \page htm_tree_gen  Tree index generation with htm_tree_gen

    The \c htm_tree_gen utility takes character separated value files
    containing points (integer ID, longitude angle and latitude angle
    columns are expected), and outputs an HTM tree index which is useful
    for fast point-in-region counts.

    \section usage Usage

    <pre><tt>
    htm_tree_gen [options] &lt;tree file&gt; &lt;input file 1&gt; [&lt;input
   file 2&gt; ...]
    </tt></pre>

    \section opts Command Line Options

    Running <tt>htm_tree_index --help</tt> provides a list of the supported
    command line options and their descriptions.

    \section over Overview

    HTM tree indexes can be used to quickly count or estimate how many points
    fall in a spherical region.

    The indexing utility produces two binary files - a data file consisting
    of points sorted in ascending HTM ID order, and a tree file which contains
    an HTM tree over the points. Tree nodes contain a count of the number of
    points inside the node and an index into the data file; empty nodes are
    omitted.

    This leads to the following simple and fast counting algorithm:

    Given a geometric region, visit all non-empty nodes overlapping the region.
    For nodes fully covered by the region, add the point count to a running
    total. For partially covered internal nodes, visit the non-empty
    children. For partially covered leaves, read points from the data
    file and check whether they are contained in the region.

    \section data Data File Algorithm

    Producing the sorted data file is conceptually simple; all that is
    required is an external sorting routine. The implementation uses
    quicksort on in-memory blocks followed by a series of k-way merges.

    \section tree Tree File Algorithm

    More difficult is an external algorithm for producing the tree file,
    especially if one wishes to optimize with respect to cache-line and
    memory page size.

    Alstrup's Split-and-Refine algorithm is used for layout. It produces
    a cache oblivious layout via repeated application of an arbitrary black-box
    algorithm, which is required to produce an optimal layout for a specified
    block size. The black-box is Clark and Munro's greedy bottom-up
    method for worst-cast optimal tree layout.

    These algorithms are detailed in the papers below:

    <pre>
    David R. Clark and J. Ian Munro.
    Efficient suffix trees on secondary storage.
    In Proceedings of the 7th Annual ACM-SIAM Symposium on Discrete Algorithms,
    Pages 383-391, Atlanta, January 1996.
    </pre>

    <pre>
    Stephen Alstrup, Michael A. Bender, Erik D. Demaine, Martin Farach-Colton,
    Theis Rauhe, and Mikkel Thorup.
    Efficient tree layout in a multilevel memory hierarchy.
    arXiv:cs.DS/0211010, 2004.
    http://www.arXiv.org/abs/cs.DS/0211010.
    </pre>

    The tree generation phase produces an HTM tree with the following
    properties:

        - Each node N corresponds to an HTM triangle, and stores
          a count of the number of entries inside N, count(N), as well
          as the index (in the data file) of its first entry.
        - If level(N) == 20, N is a leaf - no children are generated.
        - If count(N) < K for some fixed threshold K,
          N is a leaf - no children are generated.
        - If count(N) >= K and level(N) < 20, N is an internal node - the
          non-empty children of N are generated.

    Given the data file (points sorted by HTM ID), it is possible to emit
    tree nodes satisfying the above in post-order with a single sequential
    scan.

    Iterating over nodes in post-order means that a parent node is processed
    only after all children have been processed. Because Clark and Munro's
    layout algorithm is bottom-up, one can simultaneously perform layout for
    all the block-sizes used by Split-and-Refine.

    The result is a string of block IDs for each node - a unique node ID.
    Note that block IDs are handed out sequentially for a given block size.
    Since tree nodes are visited in post-order, the block ID of a parent
    must be greater than or equal to the block ID of a child. Once block IDs
    for a node have been computed at every block size, the node can be written
    to disk. If the largest block size is B, the in-memory node size is M,
    and the on disk node size is N, then 8BM/N bytes of RAM are required
    in the worst case.

    Next, the tree node file is sorted by node ID, yielding the desired
    cache-oblivious layout (per Alstrup). The sort is in ascending
    lexicographic block ID string order, always placing children before
    parents.

    At this stage, nodes contain very space-heavy node IDs, and child pointers
    are also node IDs! These are necessary for layout, but are not useful for
    tree searches. In fact, mapping a node ID to a file offset is non-trivial.

    Therefore, the sorted node file is converted to a much more space
    efficient representation:

        - Leaf nodes store a position count and the data file index of the
          first position inside the node. The index is stored as an offset
          relative to the index of the parent (this results in smaller
          numbers towards the leaves, allowing variable length coding to
          squeeze them into fewer bytes - see below).
        - Internal nodes additionally store relative tree file offsets for
          4 children. Tree file offsets of empty children are set to 0.
        - Note that internal nodes can be distinguished from leaves simply
          by comparing their position count to the leaf threshold K.

    Variable length coding is used for counts and data file indexes. Using
    such an encoding for tree file offsets is hard. Why? Because with variable
    length encoding, the size of a child offset depends on the offset value,
    which in turn depends on the size of the child offsets of nodes between
    the parent and child - in other words, it is node order dependent.
    This invalidates the block size computations required by Clark and
    Munro's algorithm, which has no a priori knowledge of the final node
    order - the reason for invoking it in the first place is to determine
    that order! Nevertheless, offsets are also variable length coded since
    it significantly reduces tree file size. The layout algorithm simply
    treats child offsets as having some fixed (hopefully close to average)
    on-disk size.

    The main difficulty during compression is the computation of tree file
    offsets from node IDs. Note that in the sorted node file a child will
    always occur before a parent. Therefore, the algorithm operates as
    follows: first, an empty hash-table that maps node IDs to file offsets
    is initialized. Next, the sorted node file is scanned sequentially.
    Finally, for each node:

        - the offsets of its children are looked up in the hashtable and the
          corresponding hashtable entries are removed (a node has exactly one
          parent).
        - the compressed byte string corresponding to the node is written
          out in reverse, and the size of the node is added to the running
          total file size.
        - an entry mapping the node id to the file size is added to the
          hashtable. To be precise: the parent of a node can subtract the
          file size just after a child was written from the current file
          size to obtain the child's offset.

    This yields a a compressed tree file T in reverse byte order. The last
    step consists of simply reversing the order of the bytes in T, giving
    the final tree file (with parent nodes always occurring before children).

    \section notes Notes

    The Clark and Munro layout algorithm is invoked only with a limited set
    of block sizes. The small number chosen (5) results in various internal
    structures having reasonable power-of-2 sizes, whilst still covering the
    block sizes that commonly occur in the multi-level memory hierarchies of
    modern 64-bit x86 systems. The largest block size used is 2 MiB, which
    corresponds to the size of a large page on x86-64. Using larger sizes
    seems to be of questionable utility and would increase the memory
    requirements for the tree generation phase, as nodes can only be written
    once a block ID for every block size has been assigned.

    The code is targeted at 64-bit machines with plenty of virtual address
    space. For example, huge files are mmapped in read-only mode (with
    MAP_NORESERVE). If system admins are restricting per-process address
    space, one may have to run `ulimit -v unlimited` prior to htm_tree_gen.

    The largest table that will have to be dealt with in the near future will
    contain around 10 billion points; at 32 bytes per record, this will
    involve asking for 320GB of address space. Should this be problematic,
    fancier IO strategies may have to be employed. Note however that e.g.
    RHEL 6 (and other recent x86-64 Linux distros) have a per process virtual
    address space limit of 128TB, and that in any case, practical tree
    generation at larger scales will require parallelization, possibly even
    across machines.

    While such parallelization is conceptually straightforward, even for
    tree generation/compression, it adds very significant implementation
    complexity, and so the code is mostly serial for now.
  */

#include <errno.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <stdexcept>
#include <sstream>

#include "../tree_entry.hxx"
#include "../sort_and_index/mem_params.hxx"
#include "../sort_and_index/tree_root.hxx"
#include "../sort_and_index/node.hxx"
#include "../sort_and_index.hxx"

size_t blk_sort_ascii (const std::vector<std::string> &infiles,
                       const std::string &outfile, const char delim,
                       const struct mem_params *const mem);

void usage (const char *prog);

int main (int argc, char **argv)
{
  std::vector<std::string> infiles;
  std::string treefile, datafile, scratch;
  size_t npoints;
  size_t memsz = 512 * 1024 * 1024;
  size_t ioblksz = 1024 * 1024;
  size_t minpoints = 1024;
  uint64_t leafthresh = 64;
  char delim = '|';

  while (1)
    {
      static struct option long_options[]
          = { { "help", no_argument, 0, 'h' },
              { "blk-size", required_argument, 0, 'b' },
              { "delim", required_argument, 0, 'd' },
              { "max-mem", required_argument, 0, 'm' },
              { "tree-min", required_argument, 0, 't' },
              { "leaf-thresh", required_argument, 0, 'l' },
              { 0, 0, 0, 0 } };
      unsigned long long v;
      char *endptr;
      int option_index = 0;
      int c = getopt_long (argc, argv, "hb:d:l:m:t:", long_options,
                           &option_index);
      if (c == -1)
        {
          break; /* no more options */
        }
      switch (c)
        {
        case 'h':
          usage (argv[0]);
          return EXIT_SUCCESS;
        case 'b':
          v = strtoull (optarg, &endptr, 0);
          if (endptr == optarg || errno != 0 || v < 1 || v > 1024 * 1024)
            {
              throw std::runtime_error (
                  "--blk-size invalid. Please specify an integer between "
                  "1 and 1,048,576 (KiB).");
            }
          ioblksz = (size_t)v * 1024;
          break;
        case 'd':
          if (strlen (optarg) != 1)
            {
              throw std::runtime_error (
                  "--delim|-d argument must be a single character");
            }
          delim = optarg[0];
          if (delim == '\0'
              || strchr ("\n.+-eE0123456789aAfFiInN", delim) != NULL)
            {
              std::stringstream ss;
              ss << "--delim invalid: '" << delim << "'";
              throw std::runtime_error (ss.str ());
            }
          break;
        case 'l':
          v = strtoull (optarg, &endptr, 0);
          if (endptr == optarg || errno != 0 || v < 1 || v > 1024 * 1024)
            {
              throw std::runtime_error (
                  "--leaf-max invalid. Please specify an integer between "
                  "1 and 1,048,576.");
            }
          leafthresh = (uint64_t)v;
          break;
        case 'm':
          v = strtoull (optarg, &endptr, 0);
          if (endptr == optarg || errno != 0 || v < 1
              || v > SIZE_MAX / 2097152)
            {
              std::stringstream ss;
              ss << "--max-mem invalid. Please specify an integer between 1 "
                    "and " << SIZE_MAX / 2097152 << " (MiB).";
              throw std::runtime_error (ss.str ());
            }
          memsz = (size_t)v * 1024 * 1024;
          break;
        case 't':
          v = strtoull (optarg, &endptr, 0);
          if (endptr == optarg || errno != 0 || v > SIZE_MAX)
            {
              std::stringstream ss;
              ss << "--tree-min invalid. Please specify a non-negative "
                    "integer less than or equal to " << SIZE_MAX;
              throw std::runtime_error (ss.str ());
            }
          minpoints = (size_t)v;
          break;
        case '?':
          return EXIT_FAILURE;
        default:
          abort ();
        }
    }
  if (argc - optind < 2)
    {
      throw std::runtime_error (
          "Output file and at least one input file must be specified");
    }

  treefile = argv[optind] + std::string (".htm");
  datafile = argv[optind] + std::string (".h5");
  scratch = argv[optind] + std::string (".scr");

  ++optind;
  for (; optind < argc; ++optind)
    {
      infiles.push_back (argv[optind]);
    }
  mem_params mem (memsz, ioblksz);

  printf ("\n");

  /* Phase 1: produce sorted point file from ASCII inputs */
  npoints = blk_sort_ascii (infiles, datafile, delim, &mem);

  sort_and_index<tree_entry>(datafile, scratch, treefile, mem, npoints,
                             minpoints, leafthresh);
  return EXIT_SUCCESS;
}
