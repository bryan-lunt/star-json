#ifndef STAR_JSON_HH
#define STAR_JSON_HH

#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <hdf5.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class star_json_t
{
  enum write_data_t { dataset, attribute };

public:
  star_json_t(const std::string file_name) :
    m_file_name(file_name)
  {
  }

  //load JSON file
  void process();

private:
  //JSON file name
  std::string m_file_name;

  //recursive iteration
  void iterate(json_t *json_object, hid_t loc_id);

  //define HDF5 type from JSON input
  hid_t create_type(const std::string &str_type, const std::vector<hsize_t> &vec_dims);

  //define HDF5 space from JSON input
  hid_t create_space(const std::string &str_type, const std::vector<hsize_t> &vec_dims);

  //store data accordingly to data type
  int write(json_t *json_value, const std::string &str_type, hid_t did, hid_t tid, write_data_t wdt);

  //define dataset corresponding to JSON array
  int handle_dataset(json_t *json_array, hid_t loc_id, const char *name_dataset);

  //define attribute corresponding to JSON array
  int handle_attribute(json_t *json_array, hid_t loc_id, const char *name_attribute);
};

#endif



