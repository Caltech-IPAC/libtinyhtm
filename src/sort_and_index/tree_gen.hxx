/*  Tree generation driver function.
 */

#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <cassert>
#include "tree_root.hxx"
#include "mem_params.hxx"
#include "now.hxx"
#include "tree_gen/emit_node.hxx"
#include "tree_gen/layout_node.hxx"
#include "tree_gen_context.hxx"

void finish_root (struct tree_root &super, tree_gen_context &ctx);

void add_node (mem_node *const root, tree_gen_context &ctx, int64_t htmid,
               int64_t count, int64_t index);

template <class T>
size_t tree_gen (const std::string &datafile, const std::string &treefile,
                 const mem_params &mem, struct tree_root &super,
                 const uint64_t leafthresh, const size_t npoints)
{
  const T *data;
  void *behind;
  int64_t htmid;
  uint64_t index, count;
  size_t i, nseg;
  int r, fd;
  const size_t pagesz = (size_t)sysconf (_SC_PAGESIZE);
  size_t mapsz = npoints * sizeof(T);
  double t;

  if (npoints == 0)
    {
      throw std::runtime_error ("no input points");
    }
  std::cout << "Generating block sorted tree node file " << treefile
            << " from " << datafile << "\n";
  t = now ();
  fd = open (datafile.c_str (), O_RDONLY);
  if (fd == -1)
    {
      throw std::runtime_error ("failed to open file " + datafile
                                + " for reading");
    }
  if (mapsz % pagesz != 0)
    {
      mapsz += pagesz - mapsz % pagesz;
    }
  behind = mmap (NULL, mapsz, PROT_READ, MAP_SHARED | MAP_NORESERVE, fd, 0);
  if (behind == MAP_FAILED)
    {
      throw std::runtime_error ("mmap() file " + datafile + " for reading");
    }
  if (madvise (behind, mapsz, MADV_SEQUENTIAL) != 0)
    {
      throw std::runtime_error ("madvise() failed on mmap for file "
                                + datafile);
    }
  data = (const T *)behind;
  behind = ((unsigned char *)behind) + mem.ioblksz;
  {
    tree_gen_context ctx (leafthresh, treefile, mem.sortsz);
    memset (&super, 0, sizeof(struct tree_root));

    /* walk over tree entries, adding tree nodes. */
    if (data[0].htmid == 0)
      {
        throw std::runtime_error ("invalid HTM ID");
      }
    htmid = 0;
    index = 0;
    count = 0;
    r = -1;

    for (i = 0; i < npoints; ++i)
      {
        if ((void *)&data[i] > behind)
          {
            void *ptr = ((unsigned char *)behind) - mem.ioblksz;
            if (madvise (ptr, mem.ioblksz, MADV_DONTNEED) != 0)
              {
                throw std::runtime_error ("madvise() failed");
              }
            behind = ((unsigned char *)behind) + mem.ioblksz;
          }
        if (data[i].htmid == htmid)
          {
            /* increase point count for the current htmid */
            ++count;
          }
        else
          {
            int r2;
            if (!(data[i].htmid > htmid))
              {
                std::stringstream ss;
                ss << "bug in tree generation phase:\n\t"
                   << "data[i].htmid: " << data[i].htmid << "\n\t"
                   << "htmid: " << htmid;
                throw std::runtime_error (ss.str ());
              }
            if (r >= 0)
              {
                /* add previous node if there is one */
                add_node (super.child[r], ctx, htmid, count, (uint64_t)index);
              }
            /* reset index, count, and htmid */
            count = 1;
            index = (uint64_t)i;
            htmid = data[i].htmid;
            r2 = (int)(htmid >> 40) - 8;
            if (r2 < 0 || r2 > 7)
              {
                throw std::runtime_error ("invalid HTM ID");
              }
            if (r != r2)
              {
                /* need a new HTM root node */
                if (r >= 0)
                  {
                    /* emit and layout the previous root if there is one */
                    emit_node (super.child[r], ctx);
                    layout_node (super.child[r], ctx);
                  }
                r = r2;
/* create new root */
#if FAST_ALLOC
                super.child[r] = (struct mem_node *)ctx.ar.alloc ();
#else
                super.child[r]
                    = (struct mem_node *)malloc (sizeof(struct mem_node));
                if (super.child[r] == NULL)
                  {
                    throw std::runtime_error ("malloc() failed");
                  }
#endif
                memset (super.child[r], 0, sizeof(struct mem_node));
                super.child[r]->htmid = r + 8;
                super.child[r]->index = index;
              }
          }
      }

    /* add last node, emit and layout last root */
    add_node (super.child[r], ctx, htmid, count, (uint64_t)index);
    emit_node (super.child[r], ctx);
    layout_node (super.child[r], ctx);

    /* assign block IDs to the HTM roots */
    finish_root (super, ctx);
    if (super.count != npoints)
      {
        std::stringstream ss;
        ss << "bug in tree generation phase:\n\t"
           << "super.count: " << super.count << "\n\t"
           << "npoints: " << npoints;
        throw std::runtime_error (ss.str ());
      }
    /* cleanup */
    i = ctx.nnodes;
    nseg = ctx.ar.nseg;
  }
  if (munmap ((void *)data, mapsz) != 0)
    {
      throw std::runtime_error ("munmap() failed");
    }
  if (close (fd) != 0)
    {
      throw std::runtime_error ("close() failed");
    }
  std::cout << "\t" << now () - t << " sec total (" << i << " tree nodes, "
            << (nseg * ARENA_SEGSZ) / (1024 * 1024) << " MiB memory)\n\n";
  return i;
}
