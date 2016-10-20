// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//
// This class implements the deconvolution step of the lattice mapping for two-particle functions.

#ifndef PHYS_LIBRARY_DCA_STEP_LATTICE_MAPPING_DECONVOLUTION_TP_H
#define PHYS_LIBRARY_DCA_STEP_LATTICE_MAPPING_DECONVOLUTION_TP_H

#include <complex>
#include <utility>

#include "dca/function/domains.hpp"
#include "dca/function/function.hpp"
#include "dca/math/function_transform/function_transform.hpp"
#include "dca/phys/domains/cluster/cluster_domain.hpp"
#include "dca/phys/domains/quantum/electron_band_domain.hpp"
#include "dca/phys/domains/time_and_frequency/vertex_frequency_domain.hpp"

#include "phys_library/DCA+_step/lattice_mapping/deconvolution/deconvolution_routines.h"

using namespace dca;
using namespace dca::phys;

namespace DCA {

template <typename parameters_type, typename source_k_dmn_t, typename target_k_dmn_t>
class deconvolution_tp
    : public deconvolution_routines<parameters_type, source_k_dmn_t, target_k_dmn_t> {
  using concurrency_type = typename parameters_type::concurrency_type;

  using compact_vertex_frequency_domain_type = domains::vertex_frequency_domain<domains::COMPACT>;
  using w_VERTEX = func::dmn_0<compact_vertex_frequency_domain_type>;
  using b = func::dmn_0<domains::electron_band_domain>;
  using host_vertex_k_cluster_type =
      domains::cluster_domain<double, parameters_type::lattice_type::DIMENSION, domains::LATTICE_TP,
                              domains::MOMENTUM_SPACE, domains::BRILLOUIN_ZONE>;
  using k_HOST_VERTEX = func::dmn_0<host_vertex_k_cluster_type>;

public:
  deconvolution_tp(parameters_type& parameters_ref);

  template <typename k_dmn_t, typename scalartype>
  void execute(func::function<std::complex<scalartype>,
                              func::dmn_variadic<func::dmn_variadic<b, b, k_dmn_t, w_VERTEX>,
                                                 func::dmn_variadic<b, b, k_dmn_t, w_VERTEX>>>&
                   Gamma_lattice_interp,
               func::function<std::complex<scalartype>,
                              func::dmn_variadic<func::dmn_variadic<b, b, target_k_dmn_t, w_VERTEX>,
                                                 func::dmn_variadic<b, b, target_k_dmn_t, w_VERTEX>>>&
                   Gamma_lattice_deconv);

private:
  parameters_type& parameters;
  concurrency_type& concurrency;
};

template <typename parameters_type, typename source_k_dmn_t, typename target_k_dmn_t>
deconvolution_tp<parameters_type, source_k_dmn_t, target_k_dmn_t>::deconvolution_tp(
    parameters_type& parameters_ref)
    : deconvolution_routines<parameters_type, source_k_dmn_t, target_k_dmn_t>(parameters_ref),

      parameters(parameters_ref),
      concurrency(parameters.get_concurrency()) {}

template <typename parameters_type, typename source_k_dmn_t, typename target_k_dmn_t>
template <typename k_dmn_t, typename scalartype>
void deconvolution_tp<parameters_type, source_k_dmn_t, target_k_dmn_t>::execute(
    func::function<std::complex<scalartype>,
                   func::dmn_variadic<func::dmn_variadic<b, b, k_dmn_t, w_VERTEX>,
                                      func::dmn_variadic<b, b, k_dmn_t, w_VERTEX>>>& Gamma_lattice_interp,
    func::function<std::complex<scalartype>,
                   func::dmn_variadic<func::dmn_variadic<b, b, target_k_dmn_t, w_VERTEX>,
                                      func::dmn_variadic<b, b, target_k_dmn_t, w_VERTEX>>>&
        Gamma_lattice_deconv) {
  int N = k_HOST_VERTEX::dmn_size();

  dca::linalg::Matrix<std::complex<scalartype>, dca::linalg::CPU> phi_inv(
      "phi_inv", std::pair<int, int>(N, N));

  this->compute_T_inv_matrix(parameters.get_singular_value_cut_off(), phi_inv);

  math::transform::FunctionTransform<k_dmn_t, target_k_dmn_t>::execute_on_all(
      Gamma_lattice_interp, Gamma_lattice_deconv, phi_inv);
}
}

#endif  // PHYS_LIBRARY_DCA_STEP_LATTICE_MAPPING_DECONVOLUTION_TP_H
