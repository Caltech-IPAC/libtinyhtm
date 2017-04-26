#pragma once

#include <cstdlib>

#include "htm.hxx"

inline struct htm_ids *_htm_ids_grow (struct htm_ids *ids)
{
  size_t cap = ids->cap;
  size_t nbytes = sizeof(struct htm_ids) + 2 * cap * sizeof(struct htm_range);

  struct htm_ids *out = (struct htm_ids *)realloc (ids, nbytes);

  if (NULL != out)
    {
      //  realloc success
      out->cap = 2 * cap;
    }
  else
    {
      //  realloc fail
      //  assume considered total failure and clean up
      free (ids);
    }

  return out;
}

inline struct htm_ids *_htm_ids_add (struct htm_ids *ids, int64_t min,
                                     int64_t max)
{
  size_t n = ids->n;
  if (n == 0)
    {
      ids->n = 1;
      ids->range[0].min = min;
      ids->range[0].max = max;
    }
  else if (min == ids->range[n - 1].max + 1)
    {
      ids->range[n - 1].max = max;
    }
  else
    {
      if (n == ids->cap)
        {
          ids = _htm_ids_grow (ids);
          if (ids == NULL)
            {
              return NULL;
            }
        }
      ids->n = n + 1;
      ids->range[n].min = min;
      ids->range[n].max = max;
    }
  return ids;
}
