#include <TipHit.h>

#include <Fragment.h>
#include <TipGeometry.h>

#include <TipPulseAnalyzer.h>

#include <cstdio>

ClassImp(TipHit)

namespace {

// ============== ParseTipChannel ==============
// Purpose: Extract the physical detector from a TIP CsI calibration name.
// Inputs: Channel name in TPCddd form.
// Outputs: Detector 1--128, or -1 for an invalid name.
int ParseTipChannel(const std::string& name) {
  if(name.size() < 6 || name.compare(0, 3, "TPC") != 0) {
    return -1;
  }
  int detector = 0;
  for(int i = 3; i < 6; ++i) {
    if(name[i] < '0' || name[i] > '9') {
      return -1;
    }
    detector = detector * 10 + name[i] - '0';
  }
  return detector >= 1 && detector <= 128 ? detector : -1;
}

}

// ============== TipHit ==============
// Purpose: Copy a TIP Fragment, resolve its position, and fit its waveform once.
// Inputs: Source TIP Fragment with its channel calibration already loaded.
// Outputs: Constructed persistent hit; a failed fit does not discard the hit.
TipHit::TipHit(const Fragment& frag)
  : fName(frag.Name()),
    fAddress(frag.Address()),
    fNumber(frag.Number()),
    fTipChannel(ParseTipChannel(fName)),
    fTimestamp(frag.Timestamp()),
    fTimestampNs(frag.TimestampNs()),
    fCFD(frag.Cfd()),
    fTime(frag.Time()),
    fCharge(frag.Charge()),
    fEnergy(frag.Energy()),
    fKValue(frag.KValue()) {
  UpdatePosition();
  FitWaveform(frag.Waveform());
}

// ============== Clear ==============
// Purpose: Restore all persistent fields to their empty-hit defaults.
// Inputs: ROOT option string, unused.
// Outputs: Cleared hit with zero PID/fit fields and zero position.
void TipHit::Clear(Option_t* opt) {
  (void)opt;
  fName.clear();
  fAddress = -1;
  fNumber = -1;
  fTipChannel = -1;
  fTimestamp = -1;
  fTimestampNs = -1;
  fCFD = -1;
  fTime = -1;
  fCharge = -1;
  fEnergy = -1;
  fKValue = -1;
  fPosition.SetXYZ(0., 0., 0.);
  fPID = 0.;
  fFitTime = 0.;
  fFitChiSq = 0;
  fFitType = 0;
}

// ============== Print ==============
// Purpose: Print the stored identity, timing, charge, geometry, and fit results.
// Inputs: ROOT option string, unused.
// Outputs: Text on standard output.
void TipHit::Print(Option_t* opt) const {
  (void)opt;
  std::printf("TIP hit: %s, address 0x%08x, number %d, TipChannel %d\n",
              fName.c_str(), fAddress, fNumber, fTipChannel);
  std::printf("  timestamp: %ld ticks, %ld ns; CFD: %d; time: %.3f ns\n",
              fTimestamp, fTimestampNs, fCFD, fTime);
  std::printf("  charge Q/K: %.6f; energy: %.6f; K: %d\n",
              fCharge, fEnergy, fKValue);
  std::printf("  position: (%.6f, %.6f, %.6f) mm\n",
              fPosition.X(), fPosition.Y(), fPosition.Z());
  std::printf("  PID: %.6f; fit time: %.6f samples; chi-square: %d; type: %d\n",
              fPID, fFitTime, fFitChiSq, fFitType);
}

// ============== UpdatePosition ==============
// Purpose: Apply the same physical-channel indexing as GRSISort.
// Inputs: Stored physical TIP channel.
// Outputs: Stored position in mm, with the geometry fallback for invalid names.
void TipHit::UpdatePosition() {
  fPosition = TipGeometry::GetPosition(fTipChannel - 1);
}

// ============== FitWaveform ==============
// Purpose: Store a single CsI fit without retaining the waveform on the hit.
// Inputs: Source waveform; empty waveforms leave the default zero fit fields.
// Outputs: Stored PID and fit quantities.
void TipHit::FitWaveform(const std::vector<Short_t>& waveform) {
  if(waveform.empty()) {
    return;
  }
  const auto fit = TipPulse::FitCsI(waveform);
  fPID = fit.pid;
  fFitTime = fit.time;
  fFitChiSq = fit.chiSq;
  fFitType = fit.type;
}
