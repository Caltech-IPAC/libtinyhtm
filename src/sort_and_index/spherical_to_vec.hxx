#ifndef SORT_AND_INDEX_SPHERICAL_TO_VEC_H
#define SORT_AND_INDEX_SPHERICAL_TO_VEC_H

// working copy - pmm

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

      const size_t num_data_elements(sizeof(T::names)/sizeof(T::names[0]));
      size_t data_size = 0; 
      int data_offset[num_data_elements];
      int data_sz[num_data_elements];
      data_offset[0] = 0;

      union {
          int8_t t8int;
          float tfloat;
          double tdouble;
      } tval;

      for(size_t i=0; i<num_data_elements; ++i) {
          data_sz[i] = T::types[i].getSize();
          if (i < num_data_elements-1) {
              data_offset[i+1] = data_offset[i] + data_sz[i];
           }
      }
      data_size = data_offset[num_data_elements-1] + T::types[num_data_elements-1].getSize();
// round data_size up to nearest multiple of 16
      if ((data_size % 16) != 0) data_size = ((data_size/16) + 1)*16;

      H5::H5File file(scratchfile, H5F_ACC_TRUNC);
      H5::CompType compound(data_size);
    
      for(size_t i=0; i<num_data_elements; ++i)
      {
        compound.insertMember(T::names[i], data_offset[i], T::types[i]);
      }

      H5::DataSet dataset(file.createDataSet("data",compound,file_space));
      std::vector<T> ra_dec(std::min(npoints,mem.memsz/sizeof(T)));
      std::vector<htm_entry<T> > htm_data(std::min(npoints,mem.memsz/sizeof(htm_entry<T>)));
      std::ifstream infile(datafile.c_str());
      hsize_t current(0);

      int64_t tmpval;

      while(current<npoints)
        {
          hsize_t n(std::min(static_cast<size_t>(npoints-current),
                             mem.memsz/sizeof(T)));
//                             mem.memsz/sizeof(htm_entry<T>)));

          infile.read(reinterpret_cast<char *>(ra_dec.data()), n*sizeof(T));
          for (size_t i=0; i<infile.gcount()/sizeof(T); ++i)
            {
/*
              htm_v3 v;
              htm_sc_tov3(&v, &ra_dec[i].sc);
              htm_data[i].v.x = v.x;
              htm_data[i].v.y = v.y;
              htm_data[i].v.z = v.z;
*/

/*
if (current == 0 && i < 5) {
    std::cout << ra_dec[i].x << " " << ra_dec[i].y << " " << ra_dec[i].z;
    std::cout << " " << ra_dec[i].mjd << " " << ra_dec[i].tsky << ra_dec[i].sso << "\n";
}
*/
     
	for(size_t j=0; j<num_data_elements; ++j)
                {
                   tmpval = ra_dec[i].data(j);

                   if (data_sz[j] == 1) {
                       tval.t8int = reinterpret_cast<int8_t &>(tmpval);
if (current == 0 && i < 2) std::cout << data_sz[j] << " " << data_offset[j] << " " << tval.t8int << "\n";
                   }
                   else if (data_sz[j] == 4) {
                       tval.tfloat = reinterpret_cast<float &>(tmpval);
if (current == 0 && i < 2) std::cout << data_sz[j] << " " << data_offset[j] << " " << tval.tfloat << "\n";
                   }
                   else if (data_sz[j] == 8) {
                       tval.tdouble = reinterpret_cast<double &>(tmpval);
if (current == 0 && i < 2) std::cout << data_sz[j] << " " << data_offset[j] << " " << tval.tdouble << "\n";
                   }

//
// way to this via reinterpret_cast - given that need to multiple elements of data[]??
//
                   memcpy(&(htm_data[i].data[data_offset[j]]), &tval, data_sz[j]);

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
