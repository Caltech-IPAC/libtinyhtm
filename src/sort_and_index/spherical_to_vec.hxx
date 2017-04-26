#pragma once

/// Phase 4: Convert spherical coords to unit vectors

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

template <class T>
void spherical_to_vec (const std::string &datafile,
                       const std::string &scratchfile,
                       const mem_params &mem_orig, const size_t npoints)
{
  double t;

  mem_params mem (mem_orig);
  mem.memsz = 1024 * 1024;

  std::cout << "Converting spherical coordinates in " + datafile
               + " to unit vectors\n";
  t = now ();

  try
    {
      H5::Exception::dontPrint ();

      hsize_t dim[] = { npoints };
      H5::DataSpace file_space (1, dim);

      const size_t num_data_elements = T::names.size ();
      size_t data_size = 0;
      int data_offset[num_data_elements];
      int data_sizes[num_data_elements];
      data_offset[0] = 0;

      for (size_t i = 0; i < num_data_elements; ++i)
        {
          data_sizes[i] = T::types[i].getSize ();
          if (i + 1 < num_data_elements)
            data_offset[i + 1] = data_offset[i] + data_sizes[i];
        }
      data_size = data_offset[num_data_elements - 1]
                  + T::types[num_data_elements - 1].getSize ();
      /// round data_size up to nearest multiple of 16
      if ((data_size % 16) != 0)
        data_size = ((data_size / 16) + 1) * 16;

      H5::H5File file (scratchfile, H5F_ACC_TRUNC);
      H5::CompType compound (data_size);

      const H5::DataType vector_type = (sizeof(typename T::vector_type) == 4
                                            ? H5::PredType::NATIVE_FLOAT
                                            : H5::PredType::NATIVE_DOUBLE);

      compound.insertMember ("x", 0, vector_type);
      compound.insertMember ("y", sizeof(typename T::vector_type),
                             vector_type);
      compound.insertMember ("z", 2 * sizeof(typename T::vector_type),
                             vector_type);

      for (size_t i = 0; i < num_data_elements; ++i)
        compound.insertMember (T::names[i], 3 * sizeof(typename T::vector_type)
                                            + data_offset[i],
                               T::types[i]);

      H5::DataSet dataset (file.createDataSet ("data", compound, file_space));
      std::vector<T> ra_dec (std::min (npoints, mem.memsz / sizeof(T)));
      std::vector<htm_entry<T> > htm_data (
          std::min (npoints, mem.memsz / sizeof(htm_entry<T>)));
      std::ifstream infile (datafile.c_str ());
      hsize_t current (0);

      while (current < npoints)
        {
          hsize_t n (std::min (static_cast<size_t>(npoints - current),
                               mem.memsz / sizeof(T)));

          infile.read (reinterpret_cast<char *>(ra_dec.data ()),
                       n * sizeof(T));
          for (size_t i = 0; i < infile.gcount () / sizeof(T); ++i)
            {
              /// Use a temporary here so that we can assign it to a
              /// double or float easily.
              htm_v3 v3;
              htm_sc_tov3 (&v3, &ra_dec[i].sc);
              htm_data[i].x = v3.x;
              htm_data[i].y = v3.y;
              htm_data[i].z = v3.z;
              for (size_t j = 0; j < num_data_elements; ++j)
                memcpy (htm_data[i].data + data_offset[j], ra_dec[i].data (j),
                        data_sizes[j]);
            }

          file_space.selectHyperslab (H5S_SELECT_SET, &n, &current);
          hsize_t mem_dim[] = { n };
          H5::DataSpace mem_space (1, mem_dim);
          dataset.write (htm_data.data (), compound, mem_space, file_space);
          current += n;
        }
    }
  catch (H5::Exception &e)
    {
      throw std::runtime_error (e.getDetailMsg ());
    }

  if (rename (scratchfile.c_str (), datafile.c_str ()) != 0)
    {
      throw std::runtime_error ("failed to rename file " + scratchfile + " to "
                                + datafile);
    }
  std::cout << "\t" << now () - t << " sec total\n\n";
}
