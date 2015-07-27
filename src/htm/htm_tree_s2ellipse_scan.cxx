#include "tinyhtm/tree.h"
#include "htm/htm_s2ellipse_cv3_template.hxx"

template <typename T>
int64_t htm_tree_s2ellipse_scan_template(const struct htm_tree *tree,
                                         const struct htm_s2ellipse *ellipse,
                                         enum htm_errcode *err,
                                         htm_callback callback)
{
    int64_t count;
    uint64_t i;

    if (tree == NULL || ellipse == NULL) {
        if (err != NULL) {
            *err = HTM_ENULLPTR;
        }
        return -1;
    }
    count = 0;
    for (i = 0, count = 0; i < tree->count; ++i) {
      if (htm_s2ellipse_cv3_template(ellipse, reinterpret_cast<T*>
                                     (static_cast<char*>
                                      (tree->entries)+i*tree->entry_size)) != 0)
        {
          if(!callback
             || callback(static_cast<char*>(tree->entries)+i*tree->entry_size))
            ++count;
        }
    }
    return count;
}


extern "C"
{
int64_t htm_tree_s2ellipse_scan(const struct htm_tree *tree,
                                const struct htm_s2ellipse *ellipse,
                                enum htm_errcode *err,
                                htm_callback callback)
{
  if(tree->element_types.at(0)==H5::PredType::NATIVE_DOUBLE)
    {
      htm_tree_s2ellipse_scan_template<double>(tree,ellipse,err,callback);
    }
  else if(tree->element_types.at(0)==H5::PredType::NATIVE_FLOAT)
    {
      htm_tree_s2ellipse_scan_template<float>(tree,ellipse,err,callback);
    }
  else
    {
      if(err!=nullptr)
        {
          *err=HTM_ETREE;
        }
      return -1;
    }
  return 0;
}
}
