// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//         Andrei Plamada    (plamada@phys.ethz.ch)
//
// Implementation of the StatisticalTesting class.

#include "dca/math/statistical_testing/statistical_testing.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <limits>
#include <stdexcept>

#include "dca/linalg/blas/blas1.hpp"
#include "dca/linalg/blas/blas2.hpp"
#include "dca/linalg/lapack/lapack.hpp"

namespace dca {
namespace math {
//  dca::math::

double StatisticalTesting::computePValue(bool known_expected_covariance, int n_samples,
                                         bool allow_fast) {
  samples_ = n_samples;
  if (allow_fast)
    computeFastMahalanobisDistanceSquared();
  else
    computeMahalanobisDistanceSquared();

  if (known_expected_covariance == true)
    pvalue_ = 1 - chi2Cdf(distance_ * n_samples, dof_);
  else
    pvalue_ = 1 - fCdf(distance_ * (double)(n_samples - dof_) / (double)dof_, dof_, n_samples - dof_);

  return pvalue_;
}

void StatisticalTesting::computeFastMahalanobisDistanceSquared() {
  if (distance_ != -1)  // already computed
    return;
  // Compute df_^T cov_^{-1} df_ = df_^T (L L^T)^{-1} df_ = (df_^T L^{-T}) (L df_) in three steps.
  // 1) triangular factorization for cov_=L L^{T};
  // 2) df_ <- df_*L^{-1};
  // 3) distance=df_^T df_

  // step 1)
  dof_ = df_.size();
  std::vector<double> cov_copy(cov_);
  try {
    dca::linalg::lapack::potrf("L", dof_, &(cov_copy[0]), dof_);
  }
  catch (dca::linalg::lapack::util::LapackException& /*err*/) {
    std::cerr << "Warning: Factorization could not be completed. Computing Mahalanobis distance in "
                 "the eigenbases.\n";
    return computeMahalanobisDistanceSquared();
  }

  // step 2)
  linalg::blas::trsv("L", "N", "N", dof_, &(cov_copy[0]), dof_, &(df_[0]), 1);
  // step 3)
  distance_ = dca::linalg::blas::dot(dof_, &(df_[0]), 1, &(df_[0]), 1);
}

void StatisticalTesting::computeMahalanobisDistanceSquared() {
  if (distance_ != -1)  // already computed
    return;

  int n = df_.size();
  std::vector<double> eigenvalues(n);
  {
    int d_worksize = 1 + 6 * n + 2 * n * n;
    std::vector<double> d_workplace(d_worksize);
    int i_worksize = 3 + 5 * n;
    std::vector<int> i_workplace(i_worksize);
    // diagonalize covariance.
    linalg::lapack::syevd("V", "U", n, &cov_[0], n, &eigenvalues[0], &d_workplace[0], d_worksize,
                          &i_workplace[0], i_worksize);
  }
  char trsp = 'T';
  std::vector<double> df_primed(n);
  // compute df in the eigenbase.
  dca::linalg::blas::gemv(&trsp, n, n, 1., &cov_[0], n, &df_[0], 1, 0., &df_primed[0], 1);
  double result = 0;
  dof_ = 0;
  const double leading = eigenvalues[n - 1];
  if (verbose_)
    std::cout << "\nLeading eigenvalue: " << leading << "\n";
  const double threshold = 10 * leading * std::numeric_limits<double>::epsilon();

  normalized_samples_.clear();
  for (int i = 0; i < n; i++)
    if (eigenvalues[i] > threshold) {
      result += df_primed[i] * df_primed[i] / eigenvalues[i];
      normalized_samples_.push_back(df_primed[i] / std::sqrt(eigenvalues[i]));
      ++dof_;
    }
    else {
      if (verbose_)
        std::cout << "Removing index " << i << "\tsigma2 " << eigenvalues[i] << "\t"
                  << "df: " << df_primed[i] << "\n";
    }
  distance_ = result;
}

void StatisticalTesting::selectIndices(std::vector<int>& indices) {
  std::sort(indices.begin(), indices.end());
  auto end = std::unique(indices.begin(), indices.end());

  using std::size_t;
  const size_t new_size = end - indices.begin();
  indices.resize(new_size);
  if (not indices.size())
    throw(std::logic_error("Test is empty."));
  if (indices[0] < 0 or indices.back() > df_.size())
    throw(std::out_of_range("Index out of bounds."));

  std::vector<double> new_df(new_size), new_cov(new_size * new_size);
  size_t i(0), j(0);
  for (auto it1 = indices.begin(); it1 < end; ++it1, ++i) {
    new_df[i] = df_[*it1];
    j = 0;
    for (auto it2 = indices.begin(); it2 < end; ++it2, ++j)
      new_cov[i * new_size + j] = cov_[(*it1) * df_.size() + (*it2)];
  }

  std::swap(new_df, df_);
  std::swap(new_cov, cov_);
}

void StatisticalTesting::discardIndices(std::vector<int>& indices) {
  std::sort(indices.begin(), indices.end());
  auto end = std::unique(indices.begin(), indices.end());
  const size_t new_size = end - indices.begin();
  indices.resize(new_size);
  if (indices[0] < 0 or indices.back() > df_.size())
    throw(std::out_of_range("Index out of bounds."));

  std::vector<int> pick;
  auto compare = indices.begin();
  for (std::size_t i = 0; i < df_.size(); ++i) {
    if (i != *compare) {
      pick.push_back(i);
    }
    else
      ++compare;
  }

  selectIndices(pick);
}

void StatisticalTesting::printInfo(const std::string& filename, bool append) const {
  if (pvalue_ == -1)
    throw(std::logic_error("StatisticalTesting: the pvalue has not been computed yet."));
  std::ofstream file(filename.c_str(), append ? std::ios::app : std::ios::trunc);
  std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  file << "# " << std::ctime(&now);
  file << "# pvalue: " << pvalue_ << "\n";
  file << "# Initial_d.o.f: " << df_.size() << "\n# final_d.o.f.: " << dof_ << "\n";
  file << "# n_samples: " << samples_ << "\n";
  file << "# normalized_samples: "
       << "\n";
  for (int i = 0; i < dof_; ++i)
    file << normalized_samples_[i] << "\n";
  file.close();
}

// Computation of the cumulative probability distributions.
constexpr double tolerance = 1e-9;
constexpr double tiny = 1e-290;

// Regularized lower incomplete gamma function, by series expansion.
// Based on
// https://github.com/lh3/samtools/blob/6bbe1609e10f27796e5bf29ac3207bb2e35ceac8/bcftools/kfunc.c
double incLGamma(double s, double z) {
  double sum, x;
  int k;
  const int limit = 200;
  for (k = 1, sum = x = 1.; k < limit; ++k) {
    sum += (x *= z / (s + k));
    if (x / sum < tolerance)
      break;
  }
  if (k == limit)
    throw(std::logic_error("incLGamma failed to converge."));

  return exp(s * log(z) - z - std::lgamma(s + 1.) + log(sum));
}

// Regularized upper incomplete gamma function, by continued fraction
static double incUGamma(double s, double z) {
  int k;
  double C, D, f;
  f = 1. + z - s;
  C = f;
  D = 0.;
  // Modified Lentz's algorithm for computing continued fraction.
  const int limit = 200;
  for (k = 1; k < limit; ++k) {
    double a = k * (s - k), b = (k << 1) + 1 + z - s, d;
    D = b + a * D;
    if (D < tiny)
      D = tiny;
    C = b + a / C;
    if (C < tiny)
      C = tiny;
    D = 1. / D;
    d = C * D;
    f *= d;
    if (fabs(d - 1.) < tolerance)
      break;
  }
  if (k == limit)
    throw(std::logic_error("incUGamma failed to converge."));

  return exp(s * log(z) - z - std::lgamma(s) - log(f));
}

// Regularized lower incomplete beta function, by series expansion.
// Based on
// https://github.com/lh3/samtools/blob/6bbe1609e10f27796e5bf29ac3207bb2e35ceac8/bcftools/kfunc.c
static double incLBeta(double a, double b, double x) {
  if (x > (a + 1.) / (a + b + 2.))  // the series expansion works only if x << 1
    return 1 - incLBeta(b, a, 1 - x);
  double C, D, f;
  if (x == 0.)
    return 0.;
  if (x == 1.)
    return 1.;
  f = 1.;
  C = f;
  D = 0.;
  // modified Lentz's algorithm for computing continued fraction
  int k;
  const int limit = 400;
  for (k = 1; k < limit; ++k) {
    double aa, d;
    int m = k >> 1;
    aa = (k & 1) ? -(a + m) * (a + b + m) * x / ((a + 2 * m) * (a + 2 * m + 1))
                 : m * (b - m) * x / ((a + 2 * m - 1) * (a + 2 * m));
    D = 1. + aa * D;
    if (D < tiny)
      D = tiny;
    C = 1. + aa / C;
    if (C < tiny)
      C = tiny;
    D = 1. / D;
    d = C * D;
    f *= d;
    if (fabs(d - 1.) < tolerance)
      break;
  }
  if (k == limit)
    throw(std::logic_error("incLBeta failed to converge."));

  return exp(std::lgamma(a + b) - std::lgamma(a) - std::lgamma(b) + a * log(x) + b * log(1. - x)) /
         a / f;
}

// Cumulative chi2 distribution
double chi2Cdf(double x, int k) {
  if (x < 0 or k <= 0)
    throw(std::logic_error("The cdf is defined only for positive arguments"));
  if (x == 0)
    return 0;

  if (x < k)
    return incLGamma(0.5 * k, 0.5 * x);

  return 1 - incUGamma(0.5 * k, 0.5 * x);
}

// Cumulative f distribution
double fCdf(double x, int nu1, int nu2) {
  if (x < 0 or nu1 <= 0 or nu2 <= 0)
    throw(std::logic_error("The cdf is defined only for positive arguments"));
  if (x == 0)
    return 0;

  return incLBeta(0.5 * nu1, 0.5 * nu2, nu1 * x / (nu1 * x + nu2));
}

}  // math
}  // dca
