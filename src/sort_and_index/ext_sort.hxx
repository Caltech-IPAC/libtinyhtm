#ifndef SORT_AND_INDEX_EXT_SORT_HXX
#define SORT_AND_INDEX_EXT_SORT_HXX

/*  Sorts the given file using multi-way external merge sort.

    @param[in] file     File to sort. Runs of size mem->sortb are assumed
                        to be sorted already.
    @param[in] tmpl     Temporary file name template.
    @param[in] mem      Memory parameters.
    @param[in] itemsz   Size of a single item in bytes.
    @param[in] nitems   Number of items in file.
 */

#include <string>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstdio>
#include <chrono>
#include "mem_params.hxx"
#include "blk_writer.hxx"
#include "ext_sort/mrg_pass.hxx"

int mrg_npasses (size_t n, size_t k);

template <class T>
void ext_sort (const std::string &file, const std::string &scratch,
               const mem_params &mem, size_t nitems)
{
  const size_t itemsz (sizeof(T));
  const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
  const size_t filesz = nitems * itemsz;
  size_t sortsz = mem.sortsz - mem.sortsz % itemsz;
  const size_t nblk = filesz / sortsz + (filesz % sortsz != 0 ? 1u : 0u);
  const int nmp = mrg_npasses (nblk, mem.k);
  int mp;

  if (nblk < 2)
    {
      std::cout << "Skipping multi-way merge step (" << file
                << " already sorted)\n";
      return;
    }
  std::cout << "Multi-way external merge sort of " << file << "\n";
  auto ttot = std::chrono::high_resolution_clock::now ();

  /* Perform k-way merge passes; the sorted block grows by a
     factor of k on every pass */
  for (mp = 0; mp < nmp; ++mp, sortsz *= mem.k)
    {
      std::string inf;
      std::string outf;
      const void *data;
      size_t nmap;
      int infd;

      std::cout << "\t- merge pass " << mp + 1 << "/" << mp << "\n";
      auto t = std::chrono::high_resolution_clock::now ();

      /* memory map input file and create writer for output file */
      if (mp % 2 == 0)
        {
          inf = std::string (file);
          outf = std::string (scratch);
        }
      else
        {
          inf = std::string (scratch);
          outf = std::string (file);
        }
      infd = open (inf.c_str (), O_RDONLY);
      if (infd == -1)
        {
          throw std::runtime_error ("failed to open file " + inf
                                    + " for reading");
        }
      if (filesz % pagesz != 0)
        {
          nmap = filesz + (pagesz - filesz % pagesz);
        }
      else
        {
          nmap = filesz;
        }
      data = mmap (NULL, nmap, PROT_READ, MAP_SHARED | MAP_NORESERVE, infd, 0);
      if (data == MAP_FAILED)
        {
          throw std::runtime_error ("failed to mmap() file " + inf);
        }
      if (madvise ((void *)data, filesz, MADV_DONTNEED) != 0)
        {
          throw std::runtime_error ("madvise() failed");
        }

      /* perform merges */
      mrg_pass<T>(outf, data, mem, filesz, sortsz);

      /* cleanup */
      if (munmap ((void *)data, nmap) != 0)
        {
          throw std::runtime_error ("failed to munmap() file " + inf);
        }
      if (close (infd) != 0)
        {
          throw std::runtime_error ("failed to close() file " + inf);
        }

      std::chrono::milliseconds msecs (
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::high_resolution_clock::now () - t));

      std::cout << msecs.count () << " msec\n";
    }
  /* make sure sorted results are in data file and delete scratch file. */
  if (nmp % 2 == 1)
    {
      if (rename (scratch.c_str (), file.c_str ()) != 0)
        {
          throw std::runtime_error ("failed to rename file "
                                    + std::string (scratch) + " to "
                                    + std::string (file));
        }
    }
  else
    {
      if (unlink (scratch.c_str ()) != 0)
        {
          throw std::runtime_error ("failed to delete file "
                                    + std::string (scratch));
        }
    }
  std::chrono::milliseconds msecs (
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::high_resolution_clock::now () - ttot));
  std::cout << "\t" << msecs.count () << " msec total\n";
}

#endif
