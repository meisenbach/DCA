// Copyright (C) 2018 ETH Zurich
// Copyright (C) 2018 UT-Battelle, LLC
// All rights reserved.
//
// See LICENSE for terms of usage.
// See CITATION.md for citation guidelines, if DCA++ is used for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// This file implements a no-change test for the two point accumulation of a mock configuration.

#include "dca/platform/dca_gpu.h"

using Scalar = double;
#include "test/mock_mcconfig.hpp"
namespace dca {
namespace config {
using McOptions = MockMcOptions<Scalar>;
}  // namespace config
}  // namespace dca

#include "test/unit/phys/dca_step/cluster_solver/test_setup.hpp"
#include "dca/phys/dca_step/cluster_solver/shared_tools/accumulation/tp/tp_accumulator_cpu.hpp"

#include <array>
#include <map>
#include <string>
#include <vector>

#include "dca/testing/gtest_h_w_warning_blocking.h"

#include "dca/function/util/difference.hpp"
#include "dca/math/random/std_random_wrapper.hpp"
#include "dca/phys/four_point_type.hpp"
#include "test/unit/phys/dca_step/cluster_solver/shared_tools/accumulation/accumulation_test.hpp"

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

constexpr bool update_baseline = false;

constexpr bool write_G4s = true;

#define INPUT_DIR \
  DCA_SOURCE_DIR "/test/unit/phys/dca_step/cluster_solver/shared_tools/accumulation/tp/"

constexpr char input_file[] = INPUT_DIR "input_4x4.json";

using ConfigGenerator = dca::testing::AccumulationTest<double>;
using Configuration = ConfigGenerator::Configuration;
using Sample = ConfigGenerator::Sample;

using Scalar = double;

using TpAccumulatorTest =
    dca::testing::G0Setup<Scalar, dca::testing::LatticeBilayer, dca::ClusterSolverId::CT_AUX, input_file>;

TEST_F(TpAccumulatorTest, Accumulate) {
  const std::array<int, 2> n{18, 22};
  Sample M;
  Configuration config;
  ConfigGenerator::prepareConfiguration(config, M, TpAccumulatorTest::BDmn::dmn_size(),
                                        TpAccumulatorTest::RDmn::dmn_size(), parameters_.get_beta(),
                                        n);
  dca::io::HDF5Writer writer;
  dca::io::HDF5Reader reader;

  const std::string baseline = INPUT_DIR "tp_accumulator_test_baseline.hdf5";

  if (update_baseline)
    writer.open_file(baseline);
  else
    reader.open_file(baseline);

  using namespace dca::phys;
  std::vector<FourPointType> four_point_channels{
      FourPointType::PARTICLE_HOLE_TRANSVERSE, FourPointType::PARTICLE_HOLE_MAGNETIC,
      FourPointType::PARTICLE_HOLE_CHARGE, FourPointType::PARTICLE_PARTICLE_UP_DOWN};

  parameters_.set_four_point_channels(four_point_channels);

  dca::phys::solver::accumulator::TpAccumulator<Parameters, dca::DistType::NONE, dca::linalg::CPU>
      accumulator(data_->G0_k_w_cluster_excluded, parameters_);

  std::map<dca::phys::FourPointType, std::string> func_names;
  func_names[dca::phys::FourPointType::PARTICLE_HOLE_TRANSVERSE] = "G4_ph_transverse";
  func_names[dca::phys::FourPointType::PARTICLE_HOLE_MAGNETIC] = "G4_ph_magnetic";
  func_names[dca::phys::FourPointType::PARTICLE_HOLE_CHARGE] = "G4_ph_charge";
  func_names[dca::phys::FourPointType::PARTICLE_PARTICLE_UP_DOWN] = "G4_pp_up_down";

  const int sign = 1;
  accumulator.accumulate(M, config, sign);
  accumulator.finalize();

  const auto& G4 = accumulator.get_G4();

  if (write_G4s) {
    dca::io::Writer writer(*adios_ptr, *concurrency_ptr, "ADIOS2", true);
    dca::io::Writer writer_h5(*adios_ptr, *concurrency_ptr, "HDF5", true);
    writer.open_file("tp_accumulator_test_G4.bp");
    writer_h5.open_file("tp_accumulator_test_G4.hdf5");
    parameters_.write(writer);
    parameters_.write(writer_h5);
    data_->write(writer);
    data_->write(writer_h5);
    for (std::size_t channel = 0; channel < accumulator.get_G4().size(); ++channel) {
      std::string channel_str = dca::phys::toString(parameters_.get_four_point_channels()[channel]);
      writer.execute("accumulator_" + channel_str, accumulator.get_G4()[channel]);
      writer_h5.execute("accumulator_" + channel_str, accumulator.get_G4()[channel]);
    }
    writer.close_file();
    writer_h5.close_file();
  }

  for(auto channel : four_point_channels) {
    Data::TpGreensFunction G4_check(func_names[channel]);
    reader.execute(G4_check);
    const auto diff = dca::func::util::difference(G4[0], G4_check);
    EXPECT_GT(1e-8, diff.l_inf);
  }
  
  if (update_baseline)
    writer.close_file();
  else
    reader.close_file();
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
