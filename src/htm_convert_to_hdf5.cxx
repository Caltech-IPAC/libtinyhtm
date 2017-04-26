#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include "htm_entry.hxx"
#include "tree_entry.hxx"

int main (int argc, char *argv[])
{
  const int mem_size (1024 * 1024);

  if (argc == 1)
    {
      std::cout << "usage: htm_convert_to_hdf5 <data file> ...\n";
      exit (1);
    }

  try
    {
      for (int arg = 1; arg < argc; ++arg)
        {
          boost::filesystem::path in_path (argv[arg]), out_path (in_path);
          out_path.replace_extension (".h5");

          size_t npoints (boost::filesystem::file_size (in_path)
                          / sizeof(htm_entry<tree_entry>));

          H5::Exception::dontPrint ();

          hsize_t dim[] = { npoints };
          H5::DataSpace file_space (1, dim);

          H5::H5File file (out_path.string (), H5F_ACC_TRUNC);
          H5::CompType compound (sizeof(htm_entry<tree_entry>));

          compound.insertMember ("x", 0, H5::PredType::NATIVE_DOUBLE);
          compound.insertMember ("y", 8, H5::PredType::NATIVE_DOUBLE);
          compound.insertMember ("z", 16, H5::PredType::NATIVE_DOUBLE);
          compound.insertMember (tree_entry::names[0], 24,
                                 tree_entry::types[0]);

          H5::DataSet dataset (
              file.createDataSet ("data", compound, file_space));

          std::vector<htm_entry<tree_entry> > htm_data (
              std::min (npoints, mem_size / sizeof(htm_entry<tree_entry>)));

          boost::filesystem::ifstream infile (in_path);
          hsize_t current (0);
          while (current < npoints)
            {
              hsize_t n (std::min (static_cast<size_t>(npoints - current),
                                   mem_size / sizeof(htm_entry<tree_entry>)));
              infile.read (reinterpret_cast<char *>(htm_data.data ()),
                           n * sizeof(htm_entry<tree_entry>));

              file_space.selectHyperslab (H5S_SELECT_SET, &n, &current);
              hsize_t mem_dim[] = { n };
              H5::DataSpace mem_space (1, mem_dim);
              dataset.write (htm_data.data (), compound, mem_space,
                             file_space);
              current += n;
            }

          boost::filesystem::path htm_path (in_path);
          htm_path.replace_extension (".htm");
          if (boost::filesystem::exists (htm_path))
            {
              hsize_t dim[] = { boost::filesystem::file_size (htm_path) };
              H5::DataSpace data_space (1, dim);
              H5::DataSet dataset (file.createDataSet (
                  "htm_index", H5::PredType::NATIVE_OPAQUE, data_space));
              boost::iostreams::mapped_file_source htm (htm_path);
              dataset.write (htm.data (), H5::PredType::NATIVE_OPAQUE);
            }
        }
    }
  catch (H5::Exception &e)
    {
      throw std::runtime_error (e.getDetailMsg ());
    }
}
