#ifndef HTM_TREE_GEN_SORT_AND_INDEX_H
#define HTM_TREE_GEN_SORT_AND_INDEX_H

#include <stddef.h>

#include "sort_and_index/mem_params.hxx"
#include "sort_and_index/tree_root.hxx"
#include "sort_and_index/tree_gen.hxx"
#include "sort_and_index/spherical_to_vec.hxx"
#include "sort_and_index/ext_sort.hxx"

uint64_t tree_compress (const std::string &treefile,
                        const std::string &scratchfile, const mem_params &mem,
                        const tree_root &super, const size_t nnodes,
                        const uint64_t leafthresh);
void reverse_file (const std::string &infile, const std::string &outfile,
                   const mem_params &mem, const uint64_t filesz);

void append_htm (const std::string &htm_path, const std::string &data_path);

template <class T>
void sort_and_index (const std::string &data_path,
                     const std::string &scratch_path,
                     const std::string &htm_path, const mem_params &mem,
                     const size_t npoints, const size_t minpoints,
                     const uint64_t leafthresh)
{
  size_t nnodes;
  ext_sort<T>(data_path, scratch_path, mem, npoints);

  bool create_index (npoints > minpoints);
  if (create_index)
    {
      struct tree_root super;
      uint64_t filesz;
      /* Phase 2: produce sorted tree nodes from data file */
      nnodes
          = tree_gen<T>(data_path, htm_path, mem, super, leafthresh, npoints);
      ext_sort<disk_node>(htm_path, scratch_path, mem, nnodes);
      /* Phase 3: compress tree file */
      filesz = tree_compress (htm_path, scratch_path, mem, super, nnodes,
                              leafthresh);
      reverse_file (scratch_path, htm_path, mem, filesz);
    }
  /* Phase 4: convert spherical coords to unit vectors */
  spherical_to_vec<T>(data_path, scratch_path, mem, npoints);

  if (create_index)
    append_htm (htm_path, data_path);
}

#endif
