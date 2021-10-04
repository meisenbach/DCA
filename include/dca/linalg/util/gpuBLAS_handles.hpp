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
// RAII wrapper for a gpu handle.

#ifndef DCA_LINALG_UTIL_CUBLAS_HANDLE_HPP
#define DCA_LINALG_UTIL_CUBLAS_HANDLE_HPP

#if defined(DCA_HAVE_CUDA)
#include #include <cublas_v2.h>
#include "dca/linalg/util/error_cublas.hpp"
#elif defined(DCA_HAVE_HIP)
#include <hipblas.h>
#include "dca/util/cuda2hip.h"
#include "dca/linalg/util/error_hipblas.hpp"
#endif

#include "dca/linalg/util/gpu_stream.hpp"

namespace dca {
namespace linalg {
namespace util {
// dca::linalg::util::

class GpuBLASHandle {
public:
  GpuBLASHandle() {
    cublasStatus_t ret = cublasCreate(&handle_);
    checkRC(ret);
  }

  GpuBLASHandle& operator=(const GpuBLASHandle& other) = delete;

  GpuBLASHandle(GpuBLASHandle&& other) {
    std::swap(handle_, other.handle_);
  }

  ~GpuBLASHandle() {
    if (handle_)
      cublasDestroy(handle_);
  }

  void setStream(cudaStream_t stream) {
    cublasStatus_t ret = cublasSetStream(handle_, stream);
    checkRC(ret);
  }

  operator cublasHandle_t() const {
    return handle_;
  }

private:
  cublasHandle_t handle_ = nullptr;
};

}  // util
}  // linalg
}  // dca

#endif  // DCA_LINALG_UTIL_GPUBLAS_HANDLE_HPP
