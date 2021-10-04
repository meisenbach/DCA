// Copyright (C) 2021 ETH Zurich
// Copyright (C) 2021 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//         Peter Doak (doakpw@ornl.gov)
//
// RAII wrapper for gpu stream.

#ifndef DCA_LINALG_UTIL_GPU_STREAM_HPP
#define DCA_LINALG_UTIL_GPU_STREAM_HPP

#include <iostream>
#if defined(DCA_HAVE_CUDA)
#include <cuda_runtime.h>
#include "dca/linalg/util/error_cuda.hpp"
#elif defined(DCA_HAVE_HIP)
#include <hip/hip_runtime.h>
#include "dca/util/cuda2hip.h"
#include "dca/linalg/util/error_hip.hpp"
#endif 

namespace dca {
namespace linalg {
namespace util {
// dca::linalg::util::

#if defined(DCA_HAVE_CUDA) || defined(DCA_HAVE_HIP)

class GpuStream {
public:
  GpuStream() {
    checkRC(cudaStreamCreate(&stream_));
  }

  GpuStream(const GpuStream& other) = delete;
  GpuStream& operator=(const GpuStream& other) = delete;

  GpuStream(GpuStream&& other) noexcept {
    swap(other);
  }

  // clang at least can't do the GpuStream_t() conversion
  cudaStream_t streamActually(){
    return stream_;
  }

  GpuStream& operator=(GpuStream&& other) noexcept {
    swap(other);
    return *this;
  }

  void sync() const {
    try {
    checkRC(cudaStreamSynchronize(stream_));
    } catch(...) {
      std::cout << "exception thrown from StreamSynchronize.\n";
    }
  }

  ~GpuStream() {
    if (stream_)
      checkRC(cudaStreamDestroy(stream_));
  }

  operator cudaStream_t() const {
    return stream_;
  }

  void swap(GpuStream& other) noexcept {
    std::swap(stream_, other.stream_);
  }

private:
  cudaStream_t stream_ = nullptr;
};

#else  // DCA_HAVE_CUDA

// Mock object.
class GpuStream {
public:
  GpuStream() = default;

  void sync() const {}

  // clang at least can't do the GpuStream_t() conversion
  auto streamActually(){
    return 0;
  }
};

#endif  // DCA_HAVE_CUDA

}  // namespace util
}  // namespace linalg
}  // namespace dca

#endif  // DCA_LINALG_UTIL_CUDA_STREAM_HPP
