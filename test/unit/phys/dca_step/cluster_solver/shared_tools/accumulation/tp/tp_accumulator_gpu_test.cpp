// Copyright (C) 2018 ETH Zurich
// Copyright (C) 2018 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// This file implements a no-change test for the two particles accumulation on the GPU.

#include "dca/config/profiler.hpp"
#include "dca/phys/dca_step/cluster_solver/shared_tools/accumulation/tp/tp_accumulator_gpu.hpp"

#include <array>
#include <functional>
#include <string>
#include "gtest/gtest.h"

#include "dca/function/util/difference.hpp"
#include "dca/math/random/std_random_wrapper.hpp"
#include "dca/phys/four_point_type.hpp"
#include "test/unit/phys/dca_step/cluster_solver/shared_tools/accumulation/accumulation_test.hpp"
#include "test/unit/phys/dca_step/cluster_solver/test_setup.hpp"

#define INPUT_DIR \
  DCA_SOURCE_DIR "/test/unit/phys/dca_step/cluster_solver/shared_tools/accumulation/tp/"

constexpr char input_file[] = INPUT_DIR "input_4x4_multitransfer_kagome.json";

#ifdef DCA_HAVE_ADIOS2
adios2::ADIOS* adios_ptr;
#endif

#ifdef DCA_HAVE_MPI
#include "dca/parallel/mpi_concurrency/mpi_concurrency.hpp"
dca::parallel::MPIConcurrency* concurrency_ptr;
#else
#include "dca/parallel/no_concurrency/no_concurrency.hpp"
dca::parallel::NoConcurrency* concurrency_ptr;
#endif

using ConfigGenerator = dca::testing::AccumulationTest<double>;
using Configuration = ConfigGenerator::Configuration;
using Sample = ConfigGenerator::Sample;

using TpAccumulatorGpuTest =
    dca::testing::G0Setup<dca::testing::LatticeKagome, dca::ClusterSolverId::CT_AUX, input_file>;

uint loop_counter = 0;

constexpr bool write_G4s = true;

TEST_F(TpAccumulatorGpuTest, Accumulate) {
  dca::linalg::util::initializeMagma();

  const std::array<int, 2> n{27, 24};
  Sample M;
  Configuration config;
  ConfigGenerator::prepareConfiguration(config, M, TpAccumulatorGpuTest::BDmn::dmn_size(),
                                        TpAccumulatorGpuTest::RDmn::dmn_size(),
                                        parameters_.get_beta(), n);

  using namespace dca::phys;
  std::vector<FourPointType> four_point_channels{
      FourPointType::PARTICLE_HOLE_TRANSVERSE, FourPointType::PARTICLE_HOLE_MAGNETIC,
      FourPointType::PARTICLE_HOLE_CHARGE, FourPointType::PARTICLE_HOLE_LONGITUDINAL_UP_UP,
      FourPointType::PARTICLE_HOLE_LONGITUDINAL_UP_DOWN, FourPointType::PARTICLE_PARTICLE_UP_DOWN};
  parameters_.set_four_point_channels(four_point_channels);

  dca::phys::solver::accumulator::TpAccumulator<Parameters, dca::DistType::NONE, dca::linalg::CPU>
      accumulatorHost(data_->G0_k_w_cluster_excluded, parameters_);
  dca::phys::solver::accumulator::TpAccumulator<Parameters, dca::DistType::NONE, dca::linalg::GPU>
      accumulatorDevice(data_->G0_k_w_cluster_excluded, parameters_);
  const int sign = 1;

  accumulatorDevice.resetAccumulation(loop_counter);
  accumulatorDevice.accumulate(M, config, sign);
  accumulatorDevice.finalize();

  accumulatorHost.resetAccumulation(loop_counter);
  accumulatorHost.accumulate(M, config, sign);
  accumulatorHost.finalize();

  ++loop_counter;

#ifdef DCA_HAVE_ADIOS2
  if (write_G4s) {
    dca::io::Writer writer(*adios_ptr, *concurrency_ptr, "ADIOS2", true);
    dca::io::Writer writer_h5(*adios_ptr, *concurrency_ptr, "HDF5", true);

    writer.open_file("tp_gpu_test_G4.bp");
    writer_h5.open_file("tp_gpu_test_G4.hdf5");

    parameters_.write(writer);
    parameters_.write(writer_h5);
    data_->write(writer);
    data_->write(writer_h5);

    for (std::size_t channel = 0; channel < accumulatorHost.get_G4().size(); ++channel) {
      std::string channel_str = dca::phys::toString(parameters_.get_four_point_channels()[channel]);
      writer.execute("accumulatorHOST_" + channel_str, accumulatorHost.get_G4()[channel]);
      writer.execute("accumulatorDevice_" + channel_str, accumulatorDevice.get_G4()[channel]);
      writer_h5.execute("accumulatorHOST_" + channel_str, accumulatorHost.get_G4()[channel]);
      writer_h5.execute("accumulatorDevice_" + channel_str, accumulatorDevice.get_G4()[channel]);
    }
    writer.close_file();
    writer_h5.close_file();
  }
#endif

  for (std::size_t channel = 0; channel < accumulatorHost.get_G4().size(); ++channel) {
    auto diff = dca::func::util::difference(accumulatorHost.get_G4()[channel],
                                            accumulatorDevice.get_G4()[channel]);
    EXPECT_GT(5e-7, diff.l_inf) << "channel: " << dca::phys::toString(four_point_channels[channel]);
  }
}

TEST_F(TpAccumulatorGpuTest, SumToAndFinalize) {
  dca::linalg::util::initializeMagma();

  parameters_.set_four_point_channel(dca::phys::FourPointType::PARTICLE_HOLE_TRANSVERSE);

  using Accumulator =
      dca::phys::solver::accumulator::TpAccumulator<G0Setup::Parameters, dca::DistType::NONE,
                                                    dca::linalg::GPU>;
  Accumulator accumulator_sum(data_->G0_k_w_cluster_excluded, parameters_, 0);
  Accumulator accumulator1(data_->G0_k_w_cluster_excluded, parameters_, 1);
  Accumulator accumulator2(data_->G0_k_w_cluster_excluded, parameters_, 2);
  Accumulator accumulator3(data_->G0_k_w_cluster_excluded, parameters_, 3);

  auto prepare_configuration = [&](auto& M, auto& configuration, const auto& n) {
    ConfigGenerator::prepareConfiguration(M, configuration, TpAccumulatorGpuTest::BDmn::dmn_size(),
                                          TpAccumulatorGpuTest::RDmn::dmn_size(),
                                          parameters_.get_beta(), n);
  };

  const std::array<int, 2> n{3, 4};
  const int sign = -1;
  Sample M1, M2;
  Configuration config1, config2;
  prepare_configuration(config1, M1, n);
  prepare_configuration(config2, M2, n);

  const int loop_id = loop_counter++;
  accumulator1.resetAccumulation(loop_id);
  accumulator2.resetAccumulation(loop_id);
  accumulator_sum.resetAccumulation(loop_id);

  accumulator1.accumulate(M1, config1, sign);
  accumulator2.accumulate(M2, config2, sign);
  accumulator1.sumTo(accumulator_sum);
  accumulator2.sumTo(accumulator_sum);
  accumulator_sum.finalize();

  // Reset the G4 on the GPU to zero.
  accumulator3.resetAccumulation(loop_counter++);
  accumulator3.accumulate(M1, config1, sign);
  accumulator3.accumulate(M2, config2, sign);
  accumulator3.finalize();

  const auto diff =
      dca::func::util::difference(accumulator3.get_G4()[0], accumulator_sum.get_G4()[0]);
  EXPECT_GT(5e-7, diff.l_inf);
}

int main(int argc, char** argv) {
#ifdef DCA_HAVE_MPI
  dca::parallel::MPIConcurrency concurrency(argc, argv);
  concurrency_ptr = &concurrency;
#else
  dca::parallel::NoConcurrency concurrency(argc, argv);
  concurrency_ptr = &concurrency;
#endif

#ifdef DCA_HAVE_ADIOS2
  // ADIOS expects MPI_COMM pointer or nullptr
  adios2::ADIOS adios("", concurrency_ptr->get(), false);
  adios_ptr = &adios;
#endif
  ::testing::InitGoogleTest(&argc, argv);

  // ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
  // delete listeners.Release(listeners.default_result_printer());
  // listeners.Append(new dca::testing::MinimalistPrinter);

  int result = RUN_ALL_TESTS();
  return result;
}
