#include <H5Cpp.h>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>

void append_htm(const std::string &htm_file, const std::string &data_file)
{
  hsize_t dim[]={boost::filesystem::file_size(htm_file)};
  H5::DataSpace file_space(1,dim);

  {
    H5::H5File file(data_file, H5F_ACC_RDWR);
    H5::DataSet dataset(file.createDataSet("htm_index",
                                           H5::PredType::NATIVE_OPAQUE,
                                           file_space));

    boost::iostreams::mapped_file_source htm(htm_file);

    dataset.write(htm.data(),H5::PredType::NATIVE_OPAQUE);
  }
  boost::filesystem::remove(htm_file);
}
