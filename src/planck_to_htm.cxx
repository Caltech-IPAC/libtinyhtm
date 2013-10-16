#include <valarray>
#include <memory>
#include <string>
#include <map>
#include <stdexcept>

#include <H5Cpp.h>

#include <CCfits/CCfits>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "lzf/lzf_filter.h"
#include "sort_and_index.hxx"

struct planck_tod_entry
{
  int64_t htmid;
  union
  {
    double d;
    int64_t i;
  } tsky;
  int64_t utc, ring;
  struct htm_sc sc;
  
  static std::string names[3];
  static H5::DataType types[3];

  int64_t data(const size_t i) const
  {
    assert(i==0 || i==1 || i==2);
    if(i==0)
      return tsky.i;
    else if(i==1)
      return utc;
    return ring;
  }

  bool operator<(const planck_tod_entry &p) const
  {
    return (htmid < p.htmid ||
            (htmid == p.htmid && tsky.d < p.tsky.d));
  }
} HTM_ALIGNED(16);


struct planck_hdf5_entry
{
  int64_t od, ring;
  double glon, glat, psi;
  int64_t healpix_2048;
  double tsky, dipole;
  int64_t utc;
};

std::string planck_tod_entry::names[3]={"TSKY", "UTC", "RING"};
H5::DataType planck_tod_entry::types[3]={H5::PredType::NATIVE_DOUBLE,
                                         H5::PredType::NATIVE_INT64,
                                         H5::PredType::NATIVE_INT64};

int main(int argc, char *argv[])
{
  const size_t memsz = sizeof(planck_tod_entry)*16*1024*1024;
  const size_t ioblksz(sizeof(planck_tod_entry)*32*1024);
  mem_params mem(memsz, ioblksz);
  const int htm_depth(20);

  if(argc<3)
    {
      std::cout << "planck_to_htm <out_path> <in_file_1> <in_file_2> ...\n";
      exit(1);
    }
  std::string out_path(argv[1]);
  std::string data_file(out_path+".h5"), scratch_file(out_path+".scr"),
    htm_file(out_path+".htm");

  try
    {
      register_lzf();
      size_t npoints(0);
      auto start_time = std::chrono::high_resolution_clock::now();
      {
        blk_writer<planck_tod_entry> out(data_file,mem.sortsz);
        for(int arg=2;arg<argc;++arg)
          {
            boost::filesystem::path path(argv[arg]);
            std::cout << "Reading "
                      << path.string() << std::endl;
            if(boost::iequals(path.extension().string(),".fits"))
              {
                CCfits::FITS fits_file(argv[arg],CCfits::Read,false);
        
                CCfits::ExtHDU& img = fits_file.extension(1); 

                CCfits::BinTable *image(dynamic_cast<CCfits::BinTable *>(&img));

                std::map< std::string,CCfits::Column * > &columns(image->column());

                CCfits::ColumnData<double> *tsky_column=
                  dynamic_cast<CCfits::ColumnData<double> *>(columns["TSKY"]);
                CCfits::ColumnData<double> *glon_column=
                  dynamic_cast<CCfits::ColumnData<double> *>(columns["GLON"]);
                CCfits::ColumnData<double> *glat_column=
                  dynamic_cast<CCfits::ColumnData<double> *>(columns["GLAT"]);
                CCfits::ColumnData<long long> *utc_column=
                  dynamic_cast<CCfits::ColumnData<long long> *>(columns["UTC"]);
                CCfits::ColumnData<short> *ring_column=
                  dynamic_cast<CCfits::ColumnData<short> *>(columns["RING"]);

                if(tsky_column==nullptr)
                  throw std::runtime_error("Can not open TSKY column");
                if(glon_column==nullptr)
                  throw std::runtime_error("Can not open GLON column");
                if(glat_column==nullptr)
                  throw std::runtime_error("Can not open GLAT column");
                if(utc_column==nullptr)
                  throw std::runtime_error("Can not open UTC column");
                if(ring_column==nullptr)
                  throw std::runtime_error("Can not open RING column");

                std::vector<std::string> keys;
                keys.push_back("TSKY");
                keys.push_back("GLON");
                keys.push_back("GLAT");
                keys.push_back("UTC");
                keys.push_back("RING");
                image->readData(true,keys);

                const size_t num_points_in_file(tsky_column->data().size());
                npoints+=num_points_in_file;
                for(size_t i=0; i<num_points_in_file; ++i)
                  {
                    planck_tod_entry entry;
                    /* CCfits data starts at 1, not 0 */
                    entry.tsky.d=tsky_column->data(i+1);
                    entry.utc=utc_column->data(i+1);
                    entry.ring=ring_column->data(i+1);

                    if(htm_sc_init(&entry.sc, glon_column->data(i+1),
                                   glat_column->data(i+1))!= HTM_OK)
                      {
                        std::stringstream ss;
                        ss << "Bad latitude or longitude in record: "
                           << i
                           << " lon: " << glon_column->data(i+1)
                           << " lat: " << glat_column->data(i+1)
                           << "\n";
                        throw std::runtime_error(ss.str());
                      }
                    struct htm_v3 v;
                    if (htm_sc_tov3(&v, &entry.sc) != HTM_OK)
                      {
                        std::stringstream ss;
                        ss << "Could not convert lon/lat to a vector in record: "
                           << i
                           << " lon: " << glon_column->data(i+1)
                           << " lat: " << glat_column->data(i+1)
                           << "\n";
                        throw std::runtime_error(ss.str());
                      }
                    entry.htmid=htm_v3_id(&v,htm_depth);
                    out.append(&entry);
                  }
              }
            else if(boost::iequals(path.extension().string(),".hdf")
                    || boost::iequals(path.extension().string(),".h5")
                    || boost::iequals(path.extension().string(),".hdf5"))
              {
                // H5::Exception::dontPrint();
                H5::H5File file(path.string(), H5F_ACC_RDONLY);
                H5::DataSet dataset = file.openDataSet("tod");                

                H5::DataSpace dataspace = dataset.getSpace();
                hsize_t size;
                dataspace.getSimpleExtentDims(&size, NULL);
                npoints+=size;
                std::vector<planck_hdf5_entry> hdf_entries(size);
                
                H5::CompType compound(sizeof(planck_hdf5_entry));
                compound.insertMember("od",HOFFSET(planck_hdf5_entry,od),
                                      H5::PredType::NATIVE_INT64);
                compound.insertMember("ring",HOFFSET(planck_hdf5_entry,ring),
                                      H5::PredType::NATIVE_INT64);
                compound.insertMember("glon",HOFFSET(planck_hdf5_entry,glon),
                                      H5::PredType::NATIVE_DOUBLE);
                compound.insertMember("glat",HOFFSET(planck_hdf5_entry,glat),
                                      H5::PredType::NATIVE_DOUBLE);
                compound.insertMember("psi",HOFFSET(planck_hdf5_entry,psi),
                                      H5::PredType::NATIVE_DOUBLE);
                compound.insertMember("healpix_2048",
                                      HOFFSET(planck_hdf5_entry,healpix_2048),
                                      H5::PredType::NATIVE_INT64);
                compound.insertMember("tsky",HOFFSET(planck_hdf5_entry,tsky),
                                      H5::PredType::NATIVE_DOUBLE);
                compound.insertMember("dipole",
                                      HOFFSET(planck_hdf5_entry,dipole),
                                      H5::PredType::NATIVE_DOUBLE);
                compound.insertMember("utc",HOFFSET(planck_hdf5_entry,utc),
                                      H5::PredType::NATIVE_INT64);

                dataset.read(hdf_entries.data(), compound);

                for(auto &hdf_entry: hdf_entries)
                  {
                    planck_tod_entry entry;
                    /* CCfits data starts at 1, not 0 */
                    entry.tsky.d=hdf_entry.tsky;
                    entry.utc=hdf_entry.utc;
                    entry.ring=hdf_entry.ring;

                    if(htm_sc_init(&entry.sc, hdf_entry.glon,
                                   hdf_entry.glat)!= HTM_OK)
                      {
                        std::stringstream ss;
                        ss << "Bad latitude or longitude in record: "
                           << " lon: " << hdf_entry.glon
                           << " lat: " << hdf_entry.glat
                           << "\n";
                        throw std::runtime_error(ss.str());
                      }
                    struct htm_v3 v;
                    if (htm_sc_tov3(&v, &entry.sc) != HTM_OK)
                      {
                        std::stringstream ss;
                        ss << "Could not convert lon/lat to a vector in record: "
                           << " lon: " << hdf_entry.glon
                           << " lat: " << hdf_entry.glat
                           << "\n";
                        throw std::runtime_error(ss.str());
                      }
                    entry.htmid=htm_v3_id(&v,htm_depth);
                    out.append(&entry);
                  }
              }
            else
              {
                throw std::runtime_error("Unknown file format: "
                                         + path.string());
              }
          }
      }

      std::chrono::milliseconds
        msecs(std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::high_resolution_clock::now() - start_time));
      std::cout << "Read files "
                << "\t" << msecs.count() << " msec total\n";

      const size_t minpoints = 1024;
      const uint64_t leafthresh = 64;
      sort_and_index<planck_tod_entry>(data_file, scratch_file,
                                       htm_file,mem,npoints,
                                       minpoints,leafthresh);
    }
  catch (H5::Exception &e)
    {
      std::cerr << "HDF5 Exception: "
                << e.getDetailMsg() << "\n";
      exit(1);
    }
  catch (CCfits::FitsException& e) 
    {
      std::cerr << "CCFits Exception: "
                << e.message() << "\n";
      exit(1);
    }
  catch (std::runtime_error& e) 
    {
      std::cerr << "Error: "
                << e.what() << "\n";
      exit(1);
    }
}
