#ifndef SORT_AND_INDEX_SPHERICAL_TO_VEC_H
#define SORT_AND_INDEX_SPHERICAL_TO_VEC_H

/* ================================================================ */
/*        Phase 4: Convert spherical coords to unit vectors         */
/* ================================================================ */

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <H5Cpp.h>
#include "mem_params.hxx"
#include "now.hxx"
#include "blk_writer.hxx"
#include "../htm_entry.hxx"

template<class T>
void spherical_to_vec(const std::string &datafile,
                      const std::string &scratchfile,
                      const mem_params &mem_orig,
                      const size_t npoints)
{
  double t;

  mem_params mem(mem_orig);
  mem.memsz=1024*1024;

  std::cout << "Converting spherical coordinates in " + datafile
    + " to unit vectors\n";
  t = now();
  
  try
    {
      H5::Exception::dontPrint();

      hsize_t dim[]={npoints};
      H5::DataSpace file_space(1,dim);

      H5::H5File file(scratchfile, H5F_ACC_TRUNC);
      H5::CompType compound(sizeof(htm_entry<T>));
    
      compound.insertMember("x", 0, H5::PredType::NATIVE_DOUBLE);
      compound.insertMember("y", 8, H5::PredType::NATIVE_DOUBLE);
      compound.insertMember("z", 16, H5::PredType::NATIVE_DOUBLE);
      const size_t num_data_elements(sizeof(T)/8-3);
      for(size_t i=0; i<num_data_elements; ++i)
        compound.insertMember(T::names[i], HOFFSET(htm_entry<T>, data) + 8*i,
                              T::types[i]);

      H5::DataSet dataset(file.createDataSet("data",compound,file_space));

      std::vector<T> ra_dec(std::min(npoints,mem.memsz/sizeof(T)));
      std::vector<htm_entry<T> > htm_data(std::min(npoints,mem.memsz/sizeof(T)));

      std::ifstream infile(datafile.c_str());
      hsize_t current(0);
      while(current<npoints)
        {
          hsize_t n(std::min(static_cast<size_t>(npoints-current),
                             mem.memsz/sizeof(T)));
          infile.read(reinterpret_cast<char *>(ra_dec.data()),
                      n*sizeof(T));
          for (size_t i=0; i<n; ++i)
            {
              htm_sc_tov3(&htm_data[i].v, &ra_dec[i].sc);
              for(size_t j=0; j<num_data_elements; ++j)
                {
                  htm_data[i].data[j].i=ra_dec[i].data(j);
                }
            }

          file_space.selectHyperslab(H5S_SELECT_SET,&n,&current);
          hsize_t mem_dim[]={n};
          H5::DataSpace mem_space(1,mem_dim);
          dataset.write(htm_data.data(),compound,mem_space,file_space);
          current+=n;
        }
    }
  catch (H5::Exception &e)
    {
      throw std::runtime_error(e.getDetailMsg());
    }

  if (rename(scratchfile.c_str(), datafile.c_str()) != 0)
    {
      throw std::runtime_error("failed to rename file " + scratchfile
                               + " to " + datafile);
    }
  std::cout << "\t"
            << now() - t
            << " sec total\n\n";
}

#endif
