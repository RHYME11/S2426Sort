#ifndef S2426_TIP_PULSE_ANALYZER_H
#define S2426_TIP_PULSE_ANALYZER_H

#include <vector>
#include <RtypesCore.h>

// Internal construction-time fitter; this helper has no ROOT persistence.
namespace TipPulse {

  struct FitResult {
    double pid{-1024.};
    double time{-1.};
    int chiSq{-1};
    int type{-1};
  };

  // ============== FitCsI ==============
  // Purpose: Fit the GRSISort fast/slow CsI models with fixed decay constants.
  // Inputs: Signed digitizer samples, with at least 50 baseline samples.
  // Outputs: PID ratio and fit quantities, or GRSISort-compatible failure values.
  FitResult FitCsI(const std::vector<Short_t>& waveform);

}

#endif
