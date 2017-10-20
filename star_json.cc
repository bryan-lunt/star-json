#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <hdf5.h>
#include <assert.h>
#include "star_json.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::process
/////////////////////////////////////////////////////////////////////////////////////////////////////

void star_json_t::process()
{
  json_error_t error;
  json_t *root;
  hid_t fid;

  if ((root = json_load_file(m_file_name.c_str(), 0, &error)) == NULL)
  {
    return;
  }

  //add .h5 extension to .json file
  std::string file_name(m_file_name);
  file_name += ".h5";

  if ((fid = H5Fcreate(file_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0)
  {

  }

  //root STAR-JSON must be object (HDF5 root group)
  assert(json_is_object(root));
  this->iterate(root, fid);

  if (H5Fclose(fid) < 0)
  {

  }

  if (root)
  {
    json_decref(root);
  }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::iterate
//recursive iteration 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void star_json_t::iterate(json_t *json_object, hid_t loc_id)
{
  const char *json_key;
  json_t *json_value;

  void *iter = json_object_iter(json_object);
  while (iter)
  {
    //obtain key and value from input parameter JSON object
    json_key = json_object_iter_key(iter);
    json_value = json_object_iter_value(iter);

    //detect the JSON value
    //1) value is a string: a group with no children
    //2) value is an object: a group with children
    //3) value is an array: a dataset or an attribute

    ///////////////////////////////////////////////////////////////////////////////////////
    //a group with no children
    //'json_key' is the group name, 'value' has the value 'group'
    ///////////////////////////////////////////////////////////////////////////////////////

    if (json_is_string(json_value))
    {
      const char *str_value = json_string_value(json_value);
      std::cout << str_value << " : " << json_key << std::endl;
      assert(strcmp(str_value, "group") == 0);

      hid_t gid;
      if ((gid = H5Gcreate2(loc_id, json_key, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
      {

      }

      if (H5Gclose(gid) < 0)
      {

      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //a group with children: further iterate
    //'json_key' is the group name, 'value' is another object
    ///////////////////////////////////////////////////////////////////////////////////////

    else if (json_is_object(json_value))
    {
      std::cout << "group: " << json_key << std::endl;

      hid_t gid;
      if ((gid = H5Gcreate2(loc_id, json_key, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
      {

      }

      this->iterate(json_value, gid);

      if (H5Gclose(gid) < 0)
      {

      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //a dataset or attribute in group, both are arrays
    //data for numerical types are arrays , data string type is a string
    //'json_key' is the dataset or attribute name
    //array position 0: JSON string, for attributes, with value "attribute"
    //array position 0: JSON object, for datasets, dataset STAR-JSON format
    ///////////////////////////////////////////////////////////////////////////////////////

    else if (json_is_array(json_value))
    {
      //detect if the array is the dataset format (array of objects) or attribute format
      json_t *value_pos_0 = json_array_get(json_value, 0);

      if (json_is_string(value_pos_0))
      {
        std::cout << "attribute: " << json_key << " : " << std::endl;

        //parameteres are the JSON array, JSON key is the attribute name
        handle_attribute(json_value, loc_id, json_key);

      }
      else if (json_is_object(value_pos_0))
      {
        std::cout << "dataset: " << json_key << " : " << std::endl;

        //parameteres are the JSON array, JSON key is the dataset name
        handle_dataset(json_value, loc_id, json_key);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //next item
    ///////////////////////////////////////////////////////////////////////////////////////

    iter = json_object_iter_next(json_object, iter);
  }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::handle_dataset
//define dataset corresponding to JSON array
//data for numerical types are arrays , data string type is a string
//'json_key' is the dataset name
//array position 0: JSON string, always with the value "dataset" 
//array position 1: JSON string, that identifies the dataset numerical type, a "STAR_TYPE"
//array position 2: JSON number, contains the HDF5 dataset rank
//array position 3: JSON array, contains the list of dimensions, that are JSON numbers
//array position 4: Contains either a JSON array or a JSON string, contains the dataset data
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json_t::handle_dataset(json_t *json_dataset_array, hid_t loc_id, const char *name_dataset)
{
  //dataset must be array
  assert(json_is_array(json_dataset_array));

  //the array constains: 
  //dataset at index 0
  //possible attributes starting at index 1
  size_t nbr_attributes = json_array_size(json_dataset_array) - 1;

  //array index 0 is the dataset
  json_t *array_pos_0 = json_array_get(json_dataset_array, 0);

  //must be object
  assert(json_is_object(array_pos_0));

  //and object's value must be array :-)
  json_t *json_dataset = json_object_get(array_pos_0, name_dataset);
  assert(json_is_array(json_dataset));

  //get array values
  json_t *value_pos_0 = json_array_get(json_dataset, 0);
  json_t *value_pos_1 = json_array_get(json_dataset, 1);
  json_t *value_pos_2 = json_array_get(json_dataset, 2);
  json_t *value_pos_3 = json_array_get(json_dataset, 3);
  json_t *value_pos_4 = json_array_get(json_dataset, 4);
  assert(json_is_string(value_pos_0)); //"dataset"
  assert(json_is_string(value_pos_1)); //STAR_TYPE
  assert(json_is_number(value_pos_2)); //rank
  assert(json_is_array(value_pos_3)); //dimensions
  assert(json_is_array(value_pos_4) || json_is_string(value_pos_4)); //data

  //array must have string "dataset" as the first item 
  std::string str_value_0(json_string_value(value_pos_0));
  assert(str_value_0.compare("dataset") == 0);

  //"STAR_TYPE"
  std::string str_value_1(json_string_value(value_pos_1));

  std::cout << "  datatype: " << str_value_1 << std::endl;

  //dimensions
  std::vector<hsize_t> vec_dims;
  size_t rank = json_array_size(value_pos_3);
  std::cout << "  rank: " << rank << std::endl;
  std::cout << "  dimensions: [";
  for (size_t idx = 0; idx < rank; idx++)
  {
    json_t *value = json_array_get(value_pos_3, idx);
    json_int_t integer = json_integer_value(value);
    vec_dims.push_back(static_cast<hsize_t>(integer));
    std::cout << vec_dims.at(idx);
    if (idx < rank - 1) std::cout << ",";
  }
  std::cout << "]" << std::endl;;

  //HDF5 identifiers 
  hid_t sid;
  hid_t tid;
  hid_t did;

  //create datatype
  if ((tid = create_type(str_value_1, vec_dims)) < 0)
  {

  }

  //create space
  if ((sid = create_space(str_value_1, vec_dims)) < 0)
  {

  }

  //create dataset 
  if ((did = H5Dcreate2(loc_id, name_dataset, tid, sid, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
  {

  }

  //write
  if (write(value_pos_4, str_value_1, did, tid, write_data_t::dataset) < 0)
  {

  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //handle further array positions (attributes for dataset)
  //start at index 1
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (nbr_attributes) std::cout << "  " << nbr_attributes << " attributes:" << std::endl;;
  for (size_t idx = 1; idx < nbr_attributes + 1; idx++)
  {
    //array index corresponding to each attribute
    json_t *array_pos_idx = json_array_get(json_dataset_array, idx);
    //must be object
    assert(json_is_object(array_pos_idx));
    //only 1 element
    assert(json_object_size(array_pos_idx) == 1);
    //returns an opaque iterator which can be used to iterate over all key - value pairs in object, 
    //or NULL if object is empty
    //object has only one element, no need to iterate
    void *iter = json_object_iter(array_pos_idx);
    //obtain key and value from JSON object
    const char *json_key = json_object_iter_key(iter);
    json_t *json_value = json_object_iter_value(iter);
    std::cout << "  name: " << json_key << std::endl;
    //value must be array
    assert(json_is_array(json_value));

    //parameteres are the JSON array, dataset identifier, and JSON key is the attribute name
    if (handle_attribute(json_value, did, json_key) < 0)
    {

    }

  }

  //close dataset 
  if (H5Dclose(did) < 0)
  {

  }

  if (H5Tclose(tid) < 0)
  {

  }

  if (H5Sclose(sid) < 0)
  {

  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::handle_attribute
//define attribute corresponding to JSON array
//data for numerical types are arrays , data string type is a string
//'json_key' is attribute name
//array position 0: JSON string, always with the value "attribute"
//array position 1: JSON string, that identifies the dataset numerical type, a "STAR_TYPE"
//array position 2: JSON number, contains the HDF5 dataset rank
//array position 3: JSON array, contains the list of dimensions, that are JSON numbers
//array position 4: Contains either a JSON array or a JSON string, contains the dataset data
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json_t::handle_attribute(json_t *json_array, hid_t loc_id, const char *name_attribute)
{
  assert(json_is_array(json_array));
  json_t *value_pos_0 = json_array_get(json_array, 0);
  json_t *value_pos_1 = json_array_get(json_array, 1);
  json_t *value_pos_2 = json_array_get(json_array, 2);
  json_t *value_pos_3 = json_array_get(json_array, 3);
  json_t *value_pos_4 = json_array_get(json_array, 4);
  assert(json_is_string(value_pos_0)); //"attribute"
  assert(json_is_string(value_pos_1)); //STAR_TYPE
  assert(json_is_number(value_pos_2)); //rank
  assert(json_is_array(value_pos_3)); //dimensions
  assert(json_is_array(value_pos_4) || json_is_string(value_pos_4)); //data

  std::string str_value_0(json_string_value(value_pos_0));
  assert(str_value_0.compare("attribute") == 0);

  //"STAR_TYPE"
  std::string str_value_1(json_string_value(value_pos_1));

  std::cout << "  datatype: " << str_value_1 << std::endl;

  //dimensions
  std::vector<hsize_t> vec_dims;
  size_t rank = json_array_size(value_pos_3);
  std::cout << "  rank: " << rank << std::endl;
  std::cout << "  dimensions: [";
  for (size_t idx = 0; idx < rank; idx++)
  {
    json_t *value = json_array_get(value_pos_3, idx);
    json_int_t integer = json_integer_value(value);
    vec_dims.push_back(static_cast<hsize_t>(integer));
    std::cout << vec_dims.at(idx);
    if (idx < rank - 1) std::cout << ",";
  }
  std::cout << "]" << std::endl;;

  //HDF5 identifiers 
  hid_t sid;
  hid_t tid;
  hid_t aid;

  //create datatype
  if ((tid = create_type(str_value_1, vec_dims)) < 0)
  {

  }

  //create space
  if ((sid = create_space(str_value_1, vec_dims)) < 0)
  {

  }

  //create attribute 
  if ((aid = H5Acreate2(loc_id, name_attribute, tid, sid, H5P_DEFAULT, H5P_DEFAULT)) < 0)
  {

  }

  //write
  if (write(value_pos_4, str_value_1, aid, tid, write_data_t::attribute) < 0)
  {

  }

  //close
  if (H5Aclose(aid) < 0)
  {

  }

  if (H5Tclose(tid) < 0)
  {

  }

  if (H5Sclose(sid) < 0)
  {

  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::create_space
//define HDF5 space from JSON input
/////////////////////////////////////////////////////////////////////////////////////////////////////

hid_t star_json_t::create_space(const std::string &str_type, const std::vector<hsize_t> &vec_dims)
{
  hid_t sid;

  if (str_type.compare("STAR_STRING") == 0)
  {
    if ((sid = H5Screate(H5S_SCALAR)) < 0)
    {
    }
  }
  else
  {
    if ((sid = H5Screate_simple(vec_dims.size(), vec_dims.data(), vec_dims.data())) < 0)
    {
    }
  }

  return sid;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::create_type
//define HDF5 type from JSON input
/////////////////////////////////////////////////////////////////////////////////////////////////////

hid_t star_json_t::create_type(const std::string &str_type, const std::vector<hsize_t> &vec_dims)
{
  hid_t tid;

  //STAR_STRING type is a 1D array of characters 
  //example
  //"attr1" : ["attribute", "STAR_STRING", 1, [3], "foo"]
  if (str_type.compare("STAR_STRING") == 0)
  {
    if ((tid = H5Tcopy(H5T_C_S1)) < 0)
    {
    }
    if (H5Tset_size(tid, static_cast<size_t>(vec_dims.at(0))) <= 0)
    {
    }
  }
  else if (str_type.compare("STAR_INT8") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_INT8)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_INT16") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_INT16)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_INT32") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_INT32)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_INT64") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_INT64)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_UINT8") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_UINT8)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_UINT16") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_UINT16)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_UINT32") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_UINT32)) < 0)
    {
    }
  }
  else if (str_type.compare("STAR_UINT64") == 0)
  {
    if ((tid = H5Tcopy(H5T_NATIVE_UINT64)) < 0)
    {
    }
  }
  else
  {
    assert(0);
  }

  return tid;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//star_json_t::write
//store data accordingly to data type
/////////////////////////////////////////////////////////////////////////////////////////////////////

int star_json_t::write(json_t *json_value, const std::string &str_type, hid_t did, hid_t tid, write_data_t wdt)
{
  //numerical data is an JSON array of numbers
  if (json_is_array(json_value))
  {
    size_t size_data = json_array_size(json_value);

    if (str_type.compare("STAR_INT32") == 0)
    {
      int32_t *buf = new int32_t[size_data];
      for (size_t idx = 0; idx < size_data; idx++)
      {
        json_t *value = json_array_get(json_value, idx);
        json_int_t integer = json_integer_value(value);
        buf[idx] = static_cast<int32_t>(integer);
      }
      if (wdt == write_data_t::dataset)
      {
        if (H5Dwrite(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf) < 0)
        {

        }
      }
      else if (wdt == write_data_t::attribute)
      {
        if (H5Awrite(did, tid, buf) < 0)
        {

        }
      }
      delete[] buf;
    }


  }
  //string data is a JSON string
  else if (json_is_string(json_value))
  {
    std::string str(json_string_value(json_value));
    if (wdt == write_data_t::dataset)
    {
      if (H5Dwrite(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, str.c_str()) < 0)
      {

      }
    }
    else if (wdt == write_data_t::attribute)
    {
      if (H5Awrite(did, tid, str.c_str()) < 0)
      {

      }
    }
  }

  return 0;
}




