// Copyright (C) 2018 ETH Zurich
// Copyright (C) 2018 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
// See CITATION.md for citation guidelines, if DCA++ is used for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//         Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// This file implements hdf5_reader.hpp.

#include "dca/io/hdf5/hdf5_reader.hpp"
#include "hdf5.h"

#include <fstream>
#include <stdexcept>

namespace dca {
namespace io {
// dca::io::

HDF5Reader::~HDF5Reader() {
  if (file_)
    close_file();
}

void HDF5Reader::open_file(std::string file_name) {
  {  // check whether the file exists ...
    std::wifstream tmp(file_name.c_str());

    try {
      if (!tmp or !tmp.good() or tmp.bad()) {
	throw std::runtime_error("Cannot open file : " + file_name);
      }
      else if (verbose_) {
	std::cout << "\n\n\topening file : " << file_name << "\n";
      }
    } catch ( const std::exception& ex ) {
      throw std::runtime_error("Cannot open file : " + file_name);
    }
  }

  file_ = std::make_unique<H5::H5File>(file_name.c_str(), H5F_ACC_RDONLY);
}

void HDF5Reader::close_file() {
  file_->close();
  file_.release();
}

std::string HDF5Reader::get_path() {
  std::string path = "/";

  for (size_t i = 0; i < paths_.size(); i++) {
    path = path + paths_[i];

    if (i < paths_.size() - 1)
      path = path + "/";
  }

  return path;
}

void HDF5Reader::begin_step() {
  if (in_step_)
    throw std::runtime_error("HDF5Writer::begin_step() called while already in step!");
  in_step_ = true;
}

void HDF5Reader::end_step() {
  if (!in_step_)
    throw std::runtime_error("HDF5Writer::end_step() called while not in step!");
  ++step_;
}

bool HDF5Reader::execute(const std::string& name, std::string& value) {
  std::string full_name = get_path() + "/" + name;
  if (!exists(full_name)) {
    return false;
  }

  H5::DataSet dataset = file_->openDataSet(full_name.c_str());
  const auto type = dataset.getDataType();

  const auto size = type.getSize();
  value.resize(size);

  dataset.read(value.data(), type);

  // Null string case.
  if (value == std::string{0})
    value = "";

  return true;
}

bool HDF5Reader::execute(const std::string& name, std::vector<std::string>& value) {
  std::string full_name = get_path() + "/" + name;
  if (!exists(full_name)) {
    return false;
  }

  H5::DataSet dataset = file_->openDataSet(name.c_str());
  auto size = readSize(full_name)[0];
  auto s_type = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);

  std::vector<char*> data(size);
  dataset.read(data.data(), s_type);

  value.resize(size);
  for (int i = 0; i < size; ++i) {
    value[i] = data[i];
  }

  // clean memory
  dataset.vlenReclaim(data.data(), s_type, dataset.getSpace());

  return true;
}

void HDF5Reader::read(const std::string& name, H5::DataType type, void* data) const {
  H5::DataSet dataset = file_->openDataSet(name.c_str());
  dataset.read(data, type);
}

bool HDF5Reader::exists(const std::string& name) const {
  return file_->nameExists(name);
}

std::vector<hsize_t> HDF5Reader::readSize(const std::string& name) const {
  H5::DataSet dataset = file_->openDataSet(name.c_str());
  H5::DataSpace dataspace = dataset.getSpace();

  int n_dims = dataspace.getSimpleExtentNdims();
  std::vector<hsize_t> dims(n_dims);
  dataspace.getSimpleExtentDims(dims.data(), nullptr);

  return dims;
}

}  // namespace io
}  // namespace dca
