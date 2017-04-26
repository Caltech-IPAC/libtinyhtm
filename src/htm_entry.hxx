#pragma once

#include "tinyhtm/geometry.h"

template <typename T> struct htm_entry
{
  typename T::vector_type x, y, z;
  char data[T::data_size];
} HTM_ALIGNED (16);
