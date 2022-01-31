// Copyright (C) 2021 ETH Zurich
// Copyright (C) 2021 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
// See CITATION.md for citation guidelines, if DCA++ is used for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//         Peter Doak (doakpw@ornl.gov)
//
// Wrapper to an instance of ADIOS2Reader, HDF5Reader or JSONReader.

#ifndef DCA_IO_READER_HPP
#define DCA_IO_READER_HPP

#include <mutex>
#include <string>
#include <variant>

#include "dca/io/hdf5/hdf5_reader.hpp"
#include "dca/io/json/json_reader.hpp"

#ifdef DCA_HAVE_ADIOS2
#include "dca/io/adios2/adios2_reader.hpp"
#endif

namespace dca::io {

template <class Concurrency>
class Reader {
public:
  // In: format. output format, HDF5 or JSON.
  // In: verbose. If true, the reader outputs a short log whenever it is executed.
  Reader(
#ifdef DCA_HAVE_ADIOS2
      const adios2::ADIOS& adios,
#endif
      const Concurrency& concurrency, const std::string& format, bool verbose = true)
      :
#ifdef DCA_HAVE_ADIOS2
        adios_(adios),
#endif
        concurrency_(concurrency) {
    if (format == "HDF5") {
      reader_.template emplace<io::HDF5Reader>(verbose);
    }
    else if (format == "JSON") {
      reader_.template emplace<io::JSONReader>(verbose);
    }
#ifdef DCA_HAVE_ADIOS2
    else if (format == "ADIOS2") {
      reader_.template emplace<io::ADIOS2Reader<Concurrency>>(&concurrency, verbose);
    }
#endif
    else {
      throw(std::logic_error("Invalid input format"));
    }
  }

  constexpr static bool is_reader = true;
  constexpr static bool is_writer = false;

  void open_file(const std::string& file_name) {
    std::visit([&](auto& var) { var.open_file(file_name); }, reader_);
  }
  void close_file() {
    std::visit([&](auto& var) { var.close_file(); }, reader_);
  }

  /** For reading input there is great utility in knowing if a group is present.
   *  It isn't an exceptional circumstance if a group is not present.
   */
  bool open_group(const std::string& new_path) {
    return std::visit([&](auto& var) -> bool { return var.open_group(new_path); }, reader_);
  }

  void close_group() {
    std::visit([&](auto& var) { var.close_group(); }, reader_);
  }

  template <class... Args>
  bool execute(Args&&... args) noexcept {
    return std::visit([&](auto& var) -> bool { return var.execute(std::forward<Args>(args)...); },
                      reader_);
  }

private:
  std::variant<io::HDF5Reader, io::JSONReader
#ifdef DCA_HAVE_ADIOS2
               ,
               io::ADIOS2Reader<Concurrency>
#endif
               >
      reader_;
#ifdef DCA_HAVE_ADIOS2
  const adios2::ADIOS& adios_;
#endif
  const Concurrency& concurrency_;
};

extern template class Reader<dca::parallel::NoConcurrency>;
#ifdef DCA_HAVE_MPI
extern template class Reader<dca::parallel::MPIConcurrency>;
#endif

}  // namespace dca::io

#endif  // DCA_IO_READER_HPP
