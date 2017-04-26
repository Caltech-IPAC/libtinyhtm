#ifndef HTM_TREE_GEN_MEM_PARAMS_H
#define HTM_TREE_GEN_MEM_PARAMS_H

#include <cstddef>
#include <unistd.h>

struct mem_params
{
  size_t memsz;   /* Max memory usage in bytes */
  size_t sortsz;  /* Size of blocks operated on by in-memory sorts */
  size_t ioblksz; /* Size of IO blocks */
  size_t k;       /* Number of merge segments in one multi-way merge pass */

  mem_params (size_t total, size_t Ioblksz)
  {
    const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
    if (total % (2 * pagesz) != 0)
      {
        total += 2 * pagesz - total % (2 * pagesz);
      }
    if (Ioblksz % pagesz != 0)
      {
        Ioblksz += pagesz - Ioblksz % pagesz;
      }
    if (total < 6 * Ioblksz)
      {
        total = 6 * Ioblksz;
      }
    memsz = total;
    sortsz = total / 2;
    ioblksz = Ioblksz;
    k = (total - 2 * Ioblksz) / (2 * Ioblksz);
  }
};

#endif
