/*  Tree compression driver function.
 */

#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cassert>
#include "../mem_params.hxx"
#include "../tree_root.hxx"
#include "../now.hxx"
#include "hash_table.hxx"
#include "../blk_writer.hxx"

uint64_t compress_node (struct hash_table *const ht,
                        blk_writer<unsigned char, false> &wr,
                        const struct disk_node *const n, const uint64_t filesz,
                        const uint64_t leafthresh);

uint64_t write_tree_header (struct hash_table *const ht,
                            blk_writer<unsigned char, false> &wr,
                            const tree_root &super, const uint64_t filesz,
                            const uint64_t leafthresh);

uint64_t tree_compress (const std::string &treefile,
                        const std::string &scratchfile, const mem_params &mem,
                        const tree_root &super, const size_t nnodes,
                        const uint64_t leafthresh)
{
  struct hash_table ht;
  const struct disk_node *data;
  void *behind;
  double t;
  size_t i;
  uint64_t filesz;
  int fd;
  const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
  size_t mapsz = nnodes * sizeof(struct disk_node);

  if (nnodes == 0)
    {
      throw std::runtime_error ("no input nodes");
    }
  t = now ();
  std::cout << "Generating reversed compressed tree node file " + scratchfile
               + " from " + treefile + "\n";
  fd = open (treefile.c_str (), O_RDONLY);
  if (fd == -1)
    {
      throw std::runtime_error ("failed to open file %s for reading"
                                + treefile);
    }
  if (mapsz % pagesz != 0)
    {
      mapsz += pagesz - mapsz % pagesz;
    }
  behind = mmap (NULL, mapsz, PROT_READ, MAP_SHARED | MAP_NORESERVE, fd, 0);
  if (behind == MAP_FAILED)
    {
      throw std::runtime_error ("mmap() file " + treefile + " for reading");
    }
  if (madvise (behind, mapsz, MADV_SEQUENTIAL) != 0)
    {
      throw std::runtime_error ("madvise() failed on mmap for file "
                                + treefile);
    }
  data = (const struct disk_node *)behind;
  behind = ((unsigned char *)behind) + mem.ioblksz;
  hash_table_init (&ht);
  {
    blk_writer<unsigned char, false> wr (scratchfile, mem.ioblksz);

    /* write nodes */
    for (i = 0, filesz = 0; i < nnodes; ++i)
      {
        if (i > 0)
          {
            assert (data[i - 1] < data[i] && "tree node file not sorted");
          }
        if ((void *)&data[i] > behind)
          {
            void *ptr = ((unsigned char *)behind) - mem.ioblksz;
            if (madvise (ptr, mem.ioblksz, MADV_DONTNEED) != 0)
              {
                throw std::runtime_error ("madvise() failed");
              }
            behind = ((unsigned char *)behind) + mem.ioblksz;
          }
        filesz = compress_node (&ht, wr, &data[i], filesz, leafthresh);
      }
    /* and tree header */
    filesz = write_tree_header (&ht, wr, super, filesz, leafthresh);
  }
  /* cleanup */
  hash_table_destroy (&ht);
  if (munmap ((void *)data, mapsz) != 0)
    {
      throw std::runtime_error ("munmap() failed");
    }
  if (close (fd) != 0)
    {
      throw std::runtime_error ("close() failed");
    }
  std::cout << "\t" << now () - t << " sec total\n\n";
  return filesz;
}
