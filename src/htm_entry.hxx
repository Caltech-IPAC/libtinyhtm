#ifndef HTM_ENTRY_HXX
#define HTM_ENTRY_HXX

#include "tinyhtm/geometry.h"

template<class T>
struct htm_entry
{
  struct htm_v3 v;   /**< Unit vector position. */
  union {
    int64_t i;
    double d;
  } data[sizeof(T)/8-3];
} HTM_ALIGNED(16);

#endif
