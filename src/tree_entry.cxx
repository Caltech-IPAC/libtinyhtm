#include "tree_entry.hxx"

std::array<std::string, 1> tree_entry::names{ { std::string ("rowId") } };
std::array<H5::DataType, 1> tree_entry::types{
  { H5::PredType::NATIVE_INT64 }
};
