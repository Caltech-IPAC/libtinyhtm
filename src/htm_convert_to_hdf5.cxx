#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "htm_entry.hxx"
#include "sort_and_index/tree_entry.hxx"

int main(int argc, char *argv[])
{
  const int mem_size(1024*1024);

  if(argc==1)
    {
      std::cout << "usage: htm_convert_to_hdf5 <data file> ...\n";
      exit(1);
    }

  try
    {
      for(int arg=1; arg<argc; ++arg)
        {
          std::string in_file(argv[arg]);
          std::string::size_type dot(in_file.rfind("."));
          std::string out_file(in_file.substr(0,dot)+".h5");

          size_t npoints(boost::filesystem::file_size(in_file)
                         /sizeof(htm_entry<tree_entry>));

          H5::Exception::dontPrint();

          hsize_t dim[]={npoints};
          H5::DataSpace file_space(1,dim);

          H5::H5File file(out_file, H5F_ACC_TRUNC);
          H5::CompType compound(sizeof(htm_entry<tree_entry>));
    
          compound.insertMember("x", 0, H5::PredType::NATIVE_DOUBLE);
          compound.insertMember("y", 8, H5::PredType::NATIVE_DOUBLE);
          compound.insertMember("z", 16, H5::PredType::NATIVE_DOUBLE);
          compound.insertMember(tree_entry::names[0], 24, tree_entry::types[0]);

          H5::DataSet dataset(file.createDataSet("htm",compound,file_space));

          std::vector<htm_entry<tree_entry> >
            htm_data(std::min(npoints,mem_size/sizeof(htm_entry<tree_entry>)));

          std::ifstream infile(in_file.c_str());
          hsize_t current(0);
          while(current<npoints)
            {
              hsize_t n(std::min(static_cast<size_t>(npoints-current),
                                 mem_size/sizeof(htm_entry<tree_entry>)));
              infile.read(reinterpret_cast<char *>(htm_data.data()),
                          n*sizeof(htm_entry<tree_entry>));

              file_space.selectHyperslab(H5S_SELECT_SET,&n,&current);
              hsize_t mem_dim[]={n};
              H5::DataSpace mem_space(1,mem_dim);
              dataset.write(htm_data.data(),compound,mem_space,file_space);
              current+=n;
            }
        }
    }
  catch (H5::Exception &e)
    {
      throw std::runtime_error(e.getDetailMsg());
    }
}
