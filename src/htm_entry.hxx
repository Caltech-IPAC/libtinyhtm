#ifndef HTM_ENTRY_HXX
#define HTM_ENTRY_HXX

#include "tinyhtm/geometry.h"

template<class T>
struct htm_entry
{
    char data[sizeof(T)];
} HTM_ALIGNED(16);

#endif
