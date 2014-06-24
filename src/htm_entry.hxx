#pragma once

#include "tinyhtm/geometry.h"

template<class T>
struct htm_entry
{
    htm_v3 v;
    char data[sizeof(T)-24];
} HTM_ALIGNED(16);
