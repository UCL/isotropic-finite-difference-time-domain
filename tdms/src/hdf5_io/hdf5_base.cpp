/**
 * @file hdf5_io.cpp
 * @authors Sam Cunliffe, William Graham
 * @brief Common HDF5 I/O methods abstracted to the base class.
 */
#include "hdf5_io/hdf5_base.h"

#include <iostream>
#include <stdexcept>

#include <H5Cpp.h>
#include <H5public.h>
#include <spdlog/spdlog.h>

std::vector<std::string> HDF5Base::get_datanames() const {
  std::vector<std::string> names;

  // iterate over all objects in the file
  for (unsigned int i = 0; i < file_->getNumObjs(); i++) {
    H5G_obj_t object_type = file_->getObjTypeByIdx(i);

    // if the current object is a H5::Dataset then grab its name
    if (object_type == H5G_DATASET) {
      H5std_string object_name = file_->getObjnameByIdx(i);
      names.push_back(object_name);
    }
  }
  return names;
}

void HDF5Base::ls() const {
  std::vector<std::string> names = this->get_datanames();
  for (auto name : names) std::cout << name << std::endl;
  return;
}

H5Dimension HDF5Base::shape_of(const std::string &dataname) const {
  // Get the dataspace (contains dimensionality info)
  H5::DataSpace dataspace = file_->openDataSet(dataname).getSpace();
  return H5Dimension(dataspace);
}

H5Dimension HDF5Base::shape_of(const std::string &group_name,
                               const std::string &dataname) const {
  // Open the group that contains the dataset
  H5::Group group = file_->openGroup(group_name);
  // Get the DataSpace for the DataSet within the group
  H5::DataSpace dataspace = group.openDataSet(dataname).getSpace();
  return H5Dimension(dataspace);
}

bool HDF5Base::flagged_MATLAB_empty(const std::string &object_path) const {
  // Attempt to fetch the object requested
  if (!file_->exists(object_path)) {
    throw std::runtime_error(filename_ + " has no object " + object_path);
  }
  hid_t object_reference = file_->getObjId(object_path);

  H5I_type_t object_type = file_->getHDFObjType(object_reference);
  H5::Attribute empty_attribute;// will point to the MATLAB_empty attribute
  if (object_type == H5I_GROUP) {
    H5::Group object = file_->openGroup(object_path);
    if (object.attrExists("MATLAB_empty")) {
      empty_attribute = object.openAttribute("MATLAB_empty");
    } else {
      // Object is not flagged as being empty
      return false;
    }
    object.close();
  } else if (object_type == H5I_DATASET) {
    // Dealing with a dataset
    H5::DataSet object = file_->openDataSet(object_path);
    if (object.attrExists("MATLAB_empty")) {
      empty_attribute = object.openAttribute("MATLAB_empty");
    } else {
      // Object is not flagged as being empty
      return false;
    }
    object.close();
  } else {
    // No other objects should be the result of MATLAB saving an empty object,
    // so throw error
    throw std::runtime_error(object_path + " is not a Group or a DataSet");
  }

  // Having extracted the dataset, attempt to read the attribute value
  uint8_t empty_bool[1];
  empty_attribute.read(H5::PredType::NATIVE_UINT8, empty_bool);
  empty_attribute.close();

  // And finally return whether this is flagged as true
  return empty_bool[0] == 1;
}
