#ifndef HTM_ENTRY_HXX
#define HTM_ENTRY_HXX

#include "tinyhtm/geometry.h"

// working copy - pmm

template<class T>
struct htm_entry
{
    htm_v3 v;
    char data[sizeof(T)-24];
} HTM_ALIGNED(16);

#endif
