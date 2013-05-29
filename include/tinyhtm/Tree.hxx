#ifndef TINYHTM_TREE_HXX
#define TINYHTM_TREE_HXX

namespace tinyhtm
{
  class Tree
  {
  public:
    struct htm_tree tree;
    Tree(const std::string &tree_file, const std::string &data_file)
    {
      enum htm_errcode ec = htm_tree_init(&tree, tree_file.c_str(),
                                          data_file.c_str());
      if(ec!=HTM_OK)
        throw Exception("Failed to init tree file or data file: "
                        + tree_file + " " + data_file);
    }
  };
}

#endif
