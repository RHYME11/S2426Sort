#ifndef S2426_TIP_HIT_H
#define S2426_TIP_HIT_H

#include <string>
#include <vector>

#include <TObject.h>
#include <TVector3.h>

class Fragment;

// One TIP CsI detection. Construction copies calibrated Fragment quantities
// and fits its waveform once; ROOT reading and getters do not repeat the fit.
class TipHit {
  public:
    // ============== TipHit ==============
    // Purpose: Create an empty hit for ROOT I/O, or build one from a Fragment.
    // Inputs: None, or the source TIP Fragment.
    // Outputs: Initialized persistent hit quantities.
    TipHit() = default;
    explicit TipHit(const Fragment& frag);

    // ============== ~TipHit ==============
    // Purpose: Release the hit.
    // Inputs: None.
    // Outputs: Destroyed hit.
    virtual ~TipHit() = default;

    // ============== Clear ==============
    // Purpose: Restore the empty-hit defaults.
    // Inputs: ROOT option string, unused.
    // Outputs: Reset hit.
    virtual void Clear(Option_t* opt="");

    // ============== Print ==============
    // Purpose: Print all stored quantities.
    // Inputs: ROOT option string, unused.
    // Outputs: Text on standard output.
    virtual void Print(Option_t* opt="") const;

    // ============== Name ==============
    // Purpose: Read the calibration channel name.
    // Inputs: None. Outputs: Stored name, e.g. TPC039N00X.
    const std::string& Name() const { return fName; }
    // ============== Address ==============
    // Purpose: Read the digitizer address.
    // Inputs: None. Outputs: Stored Fragment address.
    int Address() const { return fAddress; }
    // ============== Number ==============
    // Purpose: Read the calibration channel number.
    // Inputs: None. Outputs: Stored Channel::Number(), not a geometry index.
    int Number() const { return fNumber; }
    // ============== TipChannel ==============
    // Purpose: Read the physical TIP detector number parsed from Name().
    // Inputs: None. Outputs: 1--128, or -1 for an invalid CsI name.
    int TipChannel() const { return fTipChannel; }
    // ============== Timestamp ==============
    // Purpose: Read the raw digitizer timestamp.
    // Inputs: None. Outputs: Timestamp in digitizer ticks.
    long Timestamp() const { return fTimestamp; }
    // ============== TimestampNs ==============
    // Purpose: Read the timestamp converted by Fragment.
    // Inputs: None. Outputs: Timestamp in ns.
    long TimestampNs() const { return fTimestampNs; }
    // ============== CFD ==============
    // Purpose: Read the raw CFD word.
    // Inputs: None. Outputs: Stored Fragment::Cfd().
    int CFD() const { return fCFD; }
    // ============== Time ==============
    // Purpose: Read the CFD-based time computed by Fragment.
    // Inputs: None. Outputs: Stored time in ns, without extra time calibration.
    double Time() const { return fTime; }
    // ============== Charge ==============
    // Purpose: Read the TIP integration-normalized charge.
    // Inputs: None. Outputs: Stored Q/K from Fragment::Charge().
    double Charge() const { return fCharge; }
    // ============== Energy ==============
    // Purpose: Read the calibrated energy.
    // Inputs: None. Outputs: Fragment energy in calibration-defined units.
    double Energy() const { return fEnergy; }
    // ============== KValue ==============
    // Purpose: Read the charge integration length.
    // Inputs: None. Outputs: Stored Fragment::KValue().
    int KValue() const { return fKValue; }
    // ============== Position ==============
    // Purpose: Read TipGeometry::GetPosition(TipChannel() - 1).
    // Inputs: None. Outputs: Stored position in mm; invalid lookup is (0,0,1).
    const TVector3& Position() const { return fPosition; }
    // ============== PID ==============
    // Purpose: Read 100 * slow amplitude / fast amplitude.
    // Inputs: None. Outputs: Fit ratio, 0 without waveform, or negative failure code.
    double PID() const { return fPID; }
    // ============== FitTime ==============
    // Purpose: Read the fitted waveform onset relative to sample zero.
    // Inputs: None. Outputs: Sample index, 0 without waveform, or -1 on failure.
    double FitTime() const { return fFitTime; }
    // ============== FitChiSq ==============
    // Purpose: Read the raw residual sum of squares, following GRSISort's int API.
    // Inputs: None. Outputs: Truncated chi-square, 0 without waveform, or -1 on failure.
    int FitChiSq() const { return fFitChiSq; }
    // ============== FitType ==============
    // Purpose: Read the selected CsI model.
    // Inputs: None. Outputs: 1 fast+slow, 2 fast, 3 slow, 0 unattempted, -1 failed.
    int FitType() const { return fFitType; }

  private:
    // ============== UpdatePosition ==============
    // Purpose: Resolve the physical channel through the shared geometry table.
    // Inputs: Stored TipChannel().
    // Outputs: Updated fPosition.
    void UpdatePosition();

    // ============== FitWaveform ==============
    // Purpose: Fit a construction-time CsI waveform and store its results.
    // Inputs: Source Fragment waveform samples.
    // Outputs: Updated PID, fit time, chi-square, and model type.
    void FitWaveform(const std::vector<Short_t>& waveform);

    std::string fName;
    int fAddress{-1};
    int fNumber{-1};
    int fTipChannel{-1};
    long fTimestamp{-1};
    long fTimestampNs{-1};
    int fCFD{-1};
    double fTime{-1};
    double fCharge{-1};
    double fEnergy{-1};
    int fKValue{-1};
    TVector3 fPosition;
    double fPID{0.};
    double fFitTime{0.};
    int fFitChiSq{0};
    int fFitType{0};

  ClassDef(TipHit,1)
};

#endif
