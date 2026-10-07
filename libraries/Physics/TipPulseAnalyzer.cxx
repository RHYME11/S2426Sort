// CsI fitting adapted from GRSISort TPulseAnalyzer.cxx (SFU pulse analysis).
// MIT License
// Copyright (c) 2024 GRIFFIN
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <TipPulseAnalyzer.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace {

constexpr int BASELINE_RANGE = 50;
constexpr int FILTER = 8;
constexpr double NOISE_LEVEL = 100.;
constexpr double RC_TIME = 4510.;
constexpr double FAST_TIME = 64.3;
constexpr double SLOW_TIME = 380.;
using Matrix = std::array<std::array<long double, 4>, 4>;
using Vector = std::array<long double, 4>;

struct Shape {
  double chiSq{-1.};
  double time{-1.};
  int ndf{0};
  Vector amplitude{};
};

// ============== Determinant ==============
// Purpose: Evaluate the small determinant using GRSISort's column elimination.
// Inputs: Matrix copy and active dimension.
// Outputs: Determinant; zero for a singular matrix.
long double Determinant(Matrix matrix, int dimension) {
  if(dimension == 1) {
    return matrix[0][0];
  }
  int sign = 1;
  if(matrix[dimension - 1][dimension - 1] == 0.) {
    int column = dimension - 1;
    while(column >= 0 && matrix[dimension - 1][column] == 0.) {
      --column;
    }
    if(column < 0) {
      return 0.;
    }
    for(int row = 0; row < dimension; ++row) {
      std::swap(matrix[row][dimension - 1], matrix[row][column]);
    }
    sign = -1;
  }
  for(int column = dimension - 2; column >= 0; --column) {
    for(int row = 0; row < dimension; ++row) {
      matrix[row][column] -= matrix[row][dimension - 1] /
                            matrix[dimension - 1][dimension - 1] *
                            matrix[dimension - 1][column];
    }
  }
  return matrix[dimension - 1][dimension - 1] * sign *
         Determinant(matrix, dimension - 1);
}

// ============== Solve ==============
// Purpose: Solve the symmetric normal equations using GRSISort's Cramer method.
// Inputs: Matrix, right-hand vector, active dimension, and output vector.
// Outputs: Solution and true on success, false for singular/nonfinite systems.
bool Solve(const Matrix& matrix, const Vector& rhs, int dimension, Vector& solution) {
  const auto determinant = Determinant(matrix, dimension);
  if(determinant == 0. || !std::isfinite(determinant)) {
    return false;
  }
  for(int row = 0; row < dimension; ++row) {
    auto replaced = matrix;
    replaced[row] = rhs;
    solution[row] = Determinant(replaced, dimension) / determinant;
    if(!std::isfinite(solution[row])) {
      return false;
    }
  }
  return true;
}

// ============== ExclusionZone ==============
// Purpose: Determine the baseline end and start of the exponential fit region.
// Inputs: Waveform and output baseline/signal/peak indices.
// Outputs: True for a bounded rise-time exclusion zone, false otherwise.
bool ExclusionZone(const std::vector<Short_t>& wave, int& baselineEnd,
                   int& signalStart, int& peak) {
  const int size = static_cast<int>(wave.size());
  double baseline = 0.;
  for(int i = 0; i < BASELINE_RANGE; ++i) {
    baseline += wave[i];
  }
  baseline /= BASELINE_RANGE;

  int maximum = wave[0];
  peak = 0;
  for(int i = FILTER / 2; i < size - FILTER / 2; ++i) {
    int sum = 0;
    for(int j = i - FILTER / 2; j < i + FILTER / 2; ++j) {
      sum += wave[j];
    }
    sum /= FILTER; // Preserve GRSISort's integer smoothing for the peak search.
    if(sum > maximum) {
      maximum = sum;
      peak = i;
    }
  }

  signalStart = -1;
  for(int i = peak; i > BASELINE_RANGE; --i) {
    double sum = 0.;
    for(int j = i - FILTER / 2; j < i + FILTER / 2; ++j) {
      sum += wave[j];
    }
    if(sum / FILTER < baseline + NOISE_LEVEL) {
      signalStart = i;
      break;
    }
  }
  // The original rise-line fit reads through signalStart + 24 inclusively.
  if(signalStart < BASELINE_RANGE || signalStart + 3 * FILTER >= size) {
    return false;
  }

  Matrix matrix{};
  Vector rhs{}, solution{};
  for(int i = signalStart; i <= signalStart + 3 * FILTER; ++i) {
    matrix[0][0] += 1.;
    matrix[0][1] += i;
    matrix[1][1] += static_cast<long double>(i) * i;
    rhs[0] += wave[i];
    rhs[1] += static_cast<long double>(wave[i]) * i;
  }
  matrix[1][0] = matrix[0][1];
  baselineEnd = BASELINE_RANGE;
  if(Solve(matrix, rhs, 2, solution) && solution[1] != 0.) {
    const double crossing = std::floor((baseline - NOISE_LEVEL - solution[0]) /
                                       solution[1]);
    if(std::isfinite(crossing) && crossing >= BASELINE_RANGE &&
       crossing <= signalStart) {
      baselineEnd = static_cast<int>(crossing);
    }
  }
  return true;
}

// ============== Onset ==============
// Purpose: Find the zero of the exponential sum between baseline and peak.
// Inputs: Linear fit coefficients, effective decay constants, dimension, peak.
// Outputs: Waveform onset in samples, or -1 when no bounded root is found.
double Onset(const Vector& solution, const Vector& tau, int dimension, int peak) {
  const auto value = [&](double time) {
    double sum = 0.;
    for(int i = 1; i < dimension; ++i) {
      sum += solution[i] * std::exp(-time / tau[i]);
    }
    return sum;
  };
  double low = BASELINE_RANGE;
  double high = peak;
  double left = value(low);
  double right = value(high);
  if(!(left < 0. && right > 0.)) {
    return -1.;
  }
  for(int iteration = 0; iteration < 1000; ++iteration) {
    const double fraction = std::clamp(-left / (right - left), 0.01, 0.99);
    const double time = low + fraction * (high - low);
    const double residual = value(time);
    if(!std::isfinite(residual)) {
      return -1.;
    }
    if(std::abs(residual) <= 0.001) {
      return time;
    }
    if(residual > 0.) {
      high = time;
      right = residual;
    } else {
      low = time;
      left = residual;
    }
  }
  return -1.;
}

// ============== FitShape ==============
// Purpose: Fit baseline plus the RC, fast, and optional slow exponentials.
// Inputs: Waveform, exclusion limits, peak, dimension, and first decay constant.
// Outputs: Shape parameters; negative chi-square marks an unsuccessful trial.
Shape FitShape(const std::vector<Short_t>& wave, int baselineEnd,
               int signalStart, int peak, int dimension, double firstDecay) {
  Shape shape;
  const int size = static_cast<int>(wave.size());
  shape.ndf = baselineEnd + size - signalStart - dimension;
  if(shape.ndf <= 0) {
    return shape;
  }
  const Vector tau{0., RC_TIME, firstDecay * RC_TIME / (firstDecay + RC_TIME),
                   SLOW_TIME * RC_TIME / (SLOW_TIME + RC_TIME)};
  Matrix matrix{};
  Vector rhs{}, solution{};
  // Same geometric sums and normal equations as TPulseAnalyzer::FitCsIShape.
  const auto exponentialSum = [&](long double decay) {
    const auto logarithm = -signalStart / decay +
      std::log(1. - std::exp(-(size - signalStart) / decay)) -
      std::log(1. - std::exp(-1. / decay));
    return std::exp(logarithm);
  };
  for(int i = 1; i < dimension; ++i) {
    matrix[i][0] = matrix[0][i] = exponentialSum(tau[i]);
    matrix[i][i] = exponentialSum(tau[i] / 2.);
    for(int j = i + 1; j < dimension; ++j) {
      matrix[i][j] = matrix[j][i] = exponentialSum(tau[i] * tau[j] /
                                                  (tau[i] + tau[j]));
    }
  }
  shape.chiSq = 0.;
  for(int j = 0; j < size; ++j) {
    if(j >= baselineEnd && j < signalStart) {
      continue;
    }
    matrix[0][0] += 1.;
    rhs[0] += wave[j];
    shape.chiSq += static_cast<double>(wave[j]) * wave[j];
    if(j >= signalStart) {
      for(int i = 1; i < dimension; ++i) {
        rhs[i] += wave[j] * std::exp(-static_cast<double>(j) / tau[i]);
      }
    }
  }
  if(!Solve(matrix, rhs, dimension, solution)) {
    shape.chiSq = -1029.;
    return shape;
  }
  shape.time = Onset(solution, tau, dimension, peak);
  if(shape.time <= 0.) {
    shape.chiSq = -1031.;
    return shape;
  }
  shape.amplitude[0] = solution[0];
  for(int i = 1; i < dimension; ++i) {
    shape.amplitude[i] = solution[i] * std::exp(-shape.time / tau[i]);
    if(i >= 2) {
      shape.amplitude[i] *= -1.;
    }
    if(shape.amplitude[i] < 0.) {
      shape.chiSq = -1030.;
      return shape;
    }
  }
  for(int i = 0; i < dimension; ++i) {
    shape.chiSq -= solution[i] * rhs[i];
  }
  if(!std::isfinite(shape.chiSq) || shape.chiSq < 0.) {
    shape.chiSq = -1025.;
  }
  return shape;
}

}

// ============== FitCsI ==============
// Purpose: Select the GRSISort CsI model with smallest positive chi-square/ndf.
// Inputs: Waveform samples; decay constants and baseline rules are fixed above.
// Outputs: Stored-fit values with negative PID codes on unsuccessful fitting.
TipPulse::FitResult TipPulse::FitCsI(const std::vector<Short_t>& waveform) {
  FitResult result;
  if(waveform.size() < 10) {
    result.pid = -1.;
    return result;
  }
  if(waveform.size() < BASELINE_RANGE) {
    result.pid = -1036.;
    return result;
  }
  if(waveform.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    result.pid = -1034.;
    return result;
  }
  int baselineEnd = 0, signalStart = 0, peak = 0;
  if(!ExclusionZone(waveform, baselineEnd, signalStart, peak)) {
    result.pid = -1034.;
    return result;
  }
  std::array<Shape, 3> trials{
    FitShape(waveform, baselineEnd, signalStart, peak, 4, FAST_TIME),
    FitShape(waveform, baselineEnd, signalStart, peak, 3, FAST_TIME),
    FitShape(waveform, baselineEnd, signalStart, peak, 3, SLOW_TIME)
  };
  // GRSISort also attempts gamma-on-PIN (type 4), but its t[4] is uninitialized.
  // No PIN decay constant is defined for this CsI-only hit, so omit that trial.
  int selected = -1;
  double minimum = 1E111;
  for(int i = 0; i < 3; ++i) {
    const double reduced = trials[i].chiSq / trials[i].ndf;
    if(reduced > 0. && reduced < minimum) {
      minimum = reduced;
      selected = i;
    }
  }
  if(selected < 0) {
    return result;
  }
  auto& fit = trials[selected];
  if(selected == 2) {
    std::swap(fit.amplitude[2], fit.amplitude[3]);
  }
  result.pid = 100. * fit.amplitude[3] / fit.amplitude[2];
  result.time = fit.time;
  // Preserve the int API while avoiding undefined conversion on huge residuals.
  result.chiSq = static_cast<int>(std::min(fit.chiSq,
                               static_cast<double>(std::numeric_limits<int>::max())));
  result.type = selected + 1;
  return result;
}
