/*  Byte reverses an input file to an output file and removes the input file.
 */

#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mem_params.hxx"
#include "now.hxx"
#include "blk_writer.hxx"

void reverse_file (const std::string &infile, const std::string &outfile,
                   const mem_params &mem, const uint64_t filesz)
{
  const unsigned char *data;
  double t;
  uint64_t i, j;
  int fd;
  const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
  size_t mapsz = filesz;

  if (filesz == 0)
    {
      std::runtime_error ("cannot reverse an empty file");
    }
  std::cout << "Reversing file " + infile + " to produce " + outfile + "\n";
  t = now ();
  fd = open (infile.c_str (), O_RDONLY);
  if (fd == -1)
    {
      std::runtime_error ("failed to open file " + infile + " for reading");
    }
  if (mapsz % pagesz != 0)
    {
      mapsz += pagesz - mapsz % pagesz;
    }
  data = (const unsigned char *)mmap (NULL, mapsz, PROT_READ,
                                      MAP_SHARED | MAP_NORESERVE, fd, 0);
  if ((void *)data == MAP_FAILED)
    {
      throw std::runtime_error ("failed to mmap() file " + infile);
    }
  {
    blk_writer<unsigned char, false> wr (outfile, mem.ioblksz);
    j = mem.ioblksz * (filesz / mem.ioblksz);
    if (j == filesz)
      {
        j -= mem.ioblksz;
      }
    for (i = filesz - 1; i > 0; --i)
      {
        wr.append (&data[i], 1);
        if (i == j)
          {
            size_t sz = filesz - j;
            if (sz > mem.ioblksz)
              {
                sz = mem.ioblksz;
              }
            madvise ((void *)&data[i], sz, MADV_DONTNEED);
            j -= mem.ioblksz;
          }
      }
    wr.append (data);
  }
  if (munmap ((void *)data, mapsz) != 0)
    {
      throw std::runtime_error ("munmap() failed");
    }
  if (close (fd) != 0)
    {
      throw std::runtime_error ("close() failed");
    }
  if (unlink (infile.c_str ()) != 0)
    {
      throw std::runtime_error ("failed to delete file " + infile);
    }
  std::cout << "\t" << now () - t << " sec total\n\n";
}
