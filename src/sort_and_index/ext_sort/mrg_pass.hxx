#ifndef SORT_AND_INDEX_MRG_PASS_HXX
#define SORT_AND_INDEX_MRG_PASS_HXX

/*  Performs one multi-way merge pass.
 */

#include <cstdlib>
#include "../blk_writer.hxx"
#include "../mem_params.hxx"
#include "mrg_seg.hxx"
#include "mrg_pass/heap_up.hxx"
#include "mrg_pass/heap_down.hxx"

template <class T>
void mrg_pass (const std::string &outf, const void *const data,
               const mem_params &mem, const size_t filesz, const size_t sortsz)
{
  blk_writer<T> w (outf, mem.ioblksz);
  mrg_seg<T> *segs;
  size_t start;

  /* allocate merge segments */
  segs = (mrg_seg<T> *)malloc (mem.k * sizeof(mrg_seg<T>));
  if (segs == NULL)
    {
      throw std::runtime_error ("malloc() failed");
    }
  for (start = 0; start < filesz;)
    {
      size_t ns, end;
      /* initialize up to mem.k merge segments */
      for (ns = 0; ns < mem.k && start < filesz; ++ns, start = end)
        {
          end = (start + sortsz > filesz ? filesz : start + sortsz);

          segs[ns].init (reinterpret_cast<T *>((char *)data + start),
                         reinterpret_cast<T *>((char *)data + end),
                         mem.ioblksz);
          heap_up (segs, ns);
        }
      /* merge ns segments */
      while (ns > 0)
        {
          /* write minimum value from all ns merge segments to disk */
          w.append (segs->cur);
          if (segs->consume (mem.ioblksz, sizeof(T)) == 0)
            {
              segs[0] = segs[ns - 1];
              --ns;
            }
          heap_down (segs, ns);
        }
    }
  free (segs);
}

#endif
