#include <cstdlib>

#include "htm.hxx"

#define HTM_IDS_INIT_CAP 64

struct htm_ids *_htm_ids_init ()
{
  struct htm_ids *ids = (struct htm_ids *)malloc (
      sizeof(struct htm_ids) + HTM_IDS_INIT_CAP * sizeof(struct htm_range));
  if (ids != NULL)
    {
      ids->n = 0;
      ids->cap = HTM_IDS_INIT_CAP;
    }
  return ids;
}
