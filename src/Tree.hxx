#pragma once

namespace tinyhtm
{
class Tree
{
public:
  struct htm_tree tree;
  Tree (const std::string &data_file)
  {
    enum htm_errcode ec = htm_tree_init (&tree, data_file.c_str ());
    if (ec != HTM_OK)
      throw Exception ("Failed to init tree file or data file: " + data_file);
  }
};
}
