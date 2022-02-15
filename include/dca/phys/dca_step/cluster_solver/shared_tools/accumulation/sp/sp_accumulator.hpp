// Copyright (C) 2018 ETH Zurich
// Copyright (C) 2018 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//         Raffaele Solca' (rasolca@itp.phys.ethz.ch)
//         Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// This class measures the single-particle functions with a delayed NFFT scheme.

#ifndef DCA_PHYS_DCA_STEP_CLUSTER_SOLVER_SHARED_TOOLS_ACCUMULATION_SP_SP_ACCUMULATOR_HPP
#define DCA_PHYS_DCA_STEP_CLUSTER_SOLVER_SHARED_TOOLS_ACCUMULATION_SP_SP_ACCUMULATOR_HPP

#include <cassert>
#include <complex>
#include <memory>
#include <stdexcept>
#include <vector>

#include "dca/function/function.hpp"
#include "dca/math/nfft/dnfft_1d.hpp"
#include "dca/linalg/device_type.hpp"
#include "dca/linalg/matrix.hpp"
#include "dca/linalg/util/gpu_stream.hpp"
#include "dca/phys/error_computation_type.hpp"
#include "dca/phys/domains/quantum/electron_band_domain.hpp"
#include "dca/phys/domains/quantum/electron_spin_domain.hpp"
#include "dca/phys/domains/time_and_frequency/frequency_domain.hpp"
#include "dca/phys/domains/time_and_frequency/time_domain.hpp"

namespace dca {
namespace phys {
namespace solver {
namespace accumulator {
// dca::phys::solver::accumulator::

template <class Parameters, linalg::DeviceType device = linalg::CPU, typename Real = double>
class SpAccumulator;

template <class Parameters, typename Real>
class SpAccumulator<Parameters, linalg::CPU, Real> {
public:
  using Profiler = typename Parameters::profiler_type;
  using Scalar = Real;

protected:
  using TDmn = func::dmn_0<domains::time_domain>;
  using WDmn = func::dmn_0<domains::frequency_domain>;
  using BDmn = func::dmn_0<domains::electron_band_domain>;
  using SDmn = func::dmn_0<domains::electron_spin_domain>;
  using RDmn = typename Parameters::RClusterDmn;

  using NuDmn = func::dmn_variadic<BDmn, SDmn>;  // orbital-spin index
  using PDmn = func::dmn_variadic<BDmn, BDmn, RDmn>;

public:
  SpAccumulator(const Parameters& parameters_ref, bool accumulate_m_squared = false);

  void resetAccumulation();

  template <class Configuration>
  void accumulate(const std::array<linalg::Matrix<Real, linalg::CPU>, 2>& Ms,
                  const std::array<Configuration, 2>& configs, const int sign);

  void finalize();

  void sumTo(SpAccumulator<Parameters, linalg::CPU, Real>& other) const;

  void synchronizeCopy() {}

  const auto& get_sign_times_M_r_w() const;

  const auto& get_sign_times_M_r_w_sqr() const;

  const auto& get_single_measurement_sign_times_M_r_w();

  template <class T>
  void syncStreams(const T&) {}

  // Returns the allocated device memory in bytes.
  int deviceFingerprint() const {
    return 0;
  }

  std::vector<linalg::util::GpuStream*> get_streams() const {
    return std::vector<linalg::util::GpuStream*>();
  }

protected:
  constexpr static int oversampling = 8;
  using MFunction =
      func::function<std::complex<double>, func::dmn_variadic<NuDmn, NuDmn, RDmn, WDmn>>;
  using NfftType = math::nfft::Dnfft1D<Real, WDmn, PDmn, oversampling, math::nfft::CUBIC>;

  void finalizeFunction(std::array<NfftType, 2>& ft_objs, MFunction& function);

  const Parameters& parameters_;

  bool initialized_ = false;
  bool finalized_ = false;

  const bool accumulate_m_sqr_ = true;

  std::unique_ptr<MFunction> M_r_w_, M_r_w_sqr_;
  std::unique_ptr<MFunction> single_measurement_M_r_w_;

private:
  /** the accumulated cpu M_r_t */
  std::unique_ptr<std::array<NfftType, 2>> cached_nfft_obj_;
  /** the accumulated cpu squared M_r_t */
  std::unique_ptr<std::array<NfftType, 2>> cached_nfft_sqr_obj_;
  std::unique_ptr<std::array<NfftType, 2>> single_measurement_M_r_t_;

};

template <class Parameters, typename Real>
SpAccumulator<Parameters, linalg::CPU, Real>::SpAccumulator(const Parameters& parameters_ref,
                                                            const bool accumulate_m_sqr)
    : parameters_(parameters_ref), accumulate_m_sqr_(accumulate_m_sqr) {}

template <class Parameters, typename Real>
void SpAccumulator<Parameters, linalg::CPU, Real>::resetAccumulation() {
  cached_nfft_obj_ = std::make_unique<std::array<NfftType, 2>>();
  if (accumulate_m_sqr_)
    cached_nfft_sqr_obj_ = std::make_unique<std::array<NfftType, 2>>();

  single_measurement_M_r_t_ = std::make_unique<std::array<NfftType, 2>>();

  M_r_w_.release();
  M_r_w_sqr_.release();
  single_measurement_M_r_w_.release();
  finalized_ = false;
  initialized_ = true;
}

template <class Parameters, typename Real>
template <class Configuration>
void SpAccumulator<Parameters, linalg::CPU, Real>::accumulate(
    const std::array<linalg::Matrix<Real, linalg::CPU>, 2>& Ms,
    const std::array<Configuration, 2>& configs, const int sign) {
  if (!initialized_)
    throw(std::logic_error("The accumulator was not initialized."));

  const func::dmn_variadic<PDmn> bbr_dmn;
  const Real one_div_two_beta = 1. / (2. * parameters_.get_beta());
  //  constexpr Real epsilon = std::is_same<Real, double>::value ? 1e-16 : 1e-7;

  // Why here?  If stamping_period > 1 we still want M_r_t_ for only one configuration
  (*single_measurement_M_r_t_)[0].resetAccumulation();
  (*single_measurement_M_r_t_)[1].resetAccumulation();

  for (int s = 0; s < 2; ++s) {
    const auto& config = configs[s];
    for (int j = 0; j < config.size(); j++) {
      const int b_j = config[j].get_left_band();
      const int r_j = config[j].get_left_site();
      const Real t_j = config[j].get_tau();
      for (int i = 0; i < config.size(); i++) {
        const int b_i = config[i].get_right_band();
        const int r_i = config[i].get_right_site();
        const Real t_i = config[i].get_tau();
        const int delta_r = RDmn::parameter_type::subtract(r_j, r_i);
        const double scaled_tau = (t_i - t_j) * one_div_two_beta;  // + (i == j) * epsilon;

        const int index = bbr_dmn(b_i, b_j, delta_r);
        const Real f_val = Ms[s](i, j);

        (*cached_nfft_obj_)[s].accumulate(index, scaled_tau, sign * f_val);
        if (accumulate_m_sqr_)
          (*cached_nfft_sqr_obj_)[s].accumulate(index, scaled_tau, sign * f_val * f_val);

        (*single_measurement_M_r_t_)[s].accumulate(index, scaled_tau, sign * f_val);
      }
    }
  }
}

template <class Parameters, typename Real>
void SpAccumulator<Parameters, linalg::CPU, Real>::finalizeFunction(std::array<NfftType, 2>& ft_objs,
                                                                    MFunction& function) {
  func::function<std::complex<Real>, func::dmn_variadic<WDmn, PDmn>> tmp("tmp");
  const Real normalization = 1. / RDmn::dmn_size();

  for (int s = 0; s < 2; ++s) {
    ft_objs[s].finalize(tmp);
    for (int w_ind = 0; w_ind < WDmn::dmn_size(); w_ind++)
      for (int r_ind = 0; r_ind < RDmn::dmn_size(); r_ind++)
        for (int b2_ind = 0; b2_ind < BDmn::dmn_size(); b2_ind++)
          for (int b1_ind = 0; b1_ind < BDmn::dmn_size(); b1_ind++)
            function(b1_ind, s, b2_ind, s, r_ind, w_ind) +=
                tmp(w_ind, b1_ind, b2_ind, r_ind) * normalization;
  }
}

template <class Parameters, typename Real>
void SpAccumulator<Parameters, linalg::CPU, Real>::finalize() {
  if (finalized_)
    return;

  M_r_w_.reset(new MFunction("M_r_w"));
  finalizeFunction(*cached_nfft_obj_, *M_r_w_);

  if (accumulate_m_sqr_) {
    M_r_w_sqr_.reset(new MFunction("M_r_w_sqr"));
    finalizeFunction(*cached_nfft_sqr_obj_, *M_r_w_sqr_);
  }

  finalized_ = true;
  initialized_ = false;
}

template <class Parameters, typename Real>
void SpAccumulator<Parameters, linalg::CPU, Real>::sumTo(
    SpAccumulator<Parameters, linalg::CPU, Real>& other) const {
  if (!other.cached_nfft_obj_)
    other.cached_nfft_obj_.reset(new std::array<NfftType, 2>);
  if (!other.cached_nfft_sqr_obj_ && accumulate_m_sqr_)
    other.cached_nfft_sqr_obj_.reset(new std::array<NfftType, 2>);

  for (int s = 0; s < 2; ++s) {
    (*other.cached_nfft_obj_)[s] += (*cached_nfft_obj_)[s];
    if (accumulate_m_sqr_)
      (*other.cached_nfft_sqr_obj_)[s] += (*cached_nfft_sqr_obj_)[s];
  }
}

template <class Parameters, typename Real>
const auto& SpAccumulator<Parameters, linalg::CPU, Real>::get_sign_times_M_r_w() const {
  if (!finalized_)
    throw(std::logic_error("The accumulator was not finalized."));
  return *M_r_w_;
}

template <class Parameters, typename Real>
const auto& SpAccumulator<Parameters, linalg::CPU, Real>::get_sign_times_M_r_w_sqr() const {
  if (!finalized_)
    throw(std::logic_error("The accumulator was not finalized."));
  if (!accumulate_m_sqr_)
    throw(std::logic_error("M squared was not accumulated."));
  return *M_r_w_sqr_;
}

template <class Parameters, typename Real>
const auto& SpAccumulator<Parameters, linalg::CPU, Real>::get_single_measurement_sign_times_M_r_w() {
  single_measurement_M_r_w_.reset(new MFunction("single_function_M_r_w"));
  finalizeFunction(*single_measurement_M_r_t_, *single_measurement_M_r_w_);

  return *single_measurement_M_r_w_;
}

}  // namespace accumulator
}  // namespace solver
}  // namespace phys
}  // namespace dca

#endif  // DCA_PHYS_DCA_STEP_CLUSTER_SOLVER_SHARED_TOOLS_ACCUMULATION_SP_SP_ACCUMULATOR_HPP
