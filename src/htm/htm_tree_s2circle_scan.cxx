#include "tinyhtm/tree.h"
#include "htm/htm_v3_distance2.hxx"

template <typename T>
int64_t htm_tree_s2circle_scan_template (const struct htm_tree *tree,
                                         const struct htm_v3 *center,
                                         double radius, enum htm_errcode *err,
                                         htm_callback callback)
{
  double dist2;
  int64_t count;
  uint64_t i;

  if (tree == NULL || center == NULL)
    {
      if (err != NULL)
        {
          *err = HTM_ENULLPTR;
        }
      return -1;
    }
  if (radius < 0.0)
    {
      return 0;
    }
  else if (radius >= 180.0)
    {
      return (int64_t)tree->count;
    }
  dist2 = sin (radius * 0.5 * HTM_RAD_PER_DEG);
  dist2 = 4.0 * dist2 * dist2;
  count = 0;
  for (i = 0, count = 0; i < tree->count; ++i)
    {
      if (htm_v3_distance2 (
              center, reinterpret_cast<T *>(static_cast<char *>(tree->entries)
                                            + i * tree->entry_size)) <= dist2)
        {
          if (!callback || callback (static_cast<char *>(tree->entries)
                                     + i * tree->entry_size))
            ++count;
        }
    }
  return count;
}

extern "C" {
int64_t htm_tree_s2circle_scan (const struct htm_tree *tree,
                                const struct htm_v3 *center, double radius,
                                enum htm_errcode *err, htm_callback callback)
{
  if (tree->element_types.at (0) == H5::PredType::NATIVE_DOUBLE)
    {
      htm_tree_s2circle_scan_template<double>(tree, center, radius, err,
                                              callback);
    }
  else if (tree->element_types.at (0) == H5::PredType::NATIVE_FLOAT)
    {
      htm_tree_s2circle_scan_template<float>(tree, center, radius, err,
                                             callback);
    }
  else
    {
      if (err != nullptr)
        {
          *err = HTM_ETREE;
        }
      return -1;
    }
  return 0;
}
}
