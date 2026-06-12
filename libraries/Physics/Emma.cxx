
#include <Emma.h>
#include <Fragment.h>

#include <algorithm>
#include <cstdio>
#include <limits>


// ============== EmmaHit ==============
// purpose: Copy the reduced EMMA hit quantities from a fragment.
// inputs: source fragment
// outputs: initialized EmmaHit
EmmaHit::EmmaHit(const Fragment &frag) {
  fAddress = frag.Address();
  fNumber = frag.Number();
  fTimestamp = frag.Timestamp();
  fTimestampNs = frag.TimestampNs();
  fTime = frag.Time();
  fCharge = frag.Charge();
  fEnergy = frag.Energy();
}

// ============== Clear ==============
// purpose: Reset all stored hit quantities.
// inputs: ROOT option string
// outputs: none
void EmmaHit::Clear(Option_t *opt) {
  fAddress = -1;
  fNumber = -1;
  fTimestamp = 0;
  fTimestampNs = 0;
  fTime = 0;
  fCharge = 0;
  fEnergy = 0;
}

// ============== Print ==============
// purpose: Print a compact EMMA hit summary.
// inputs: ROOT option string
// outputs: none
void EmmaHit::Print(Option_t *opt) const {
  printf("address: 0x%08x number: %i charge: %.3f energy: %.3f timestamp: %ldns\n",
      fAddress, fNumber, fCharge, fEnergy, fTimestampNs);
}

// ============== Clear ==============
// purpose: Clear stored EMMA hits and derived quantities.
// inputs: ROOT option string
// outputs: none
void Emma::Clear(Option_t *opt) { 
  fADC.clear();
  fTDC.clear();
  fSi.clear();
  fIC1.clear();
  fIC2.clear();
  fIC3.clear();
  fIC4.clear();
  fAnodes.clear();
  fLeft.clear();
  fRight.clear();
  fTop.clear();
  fBot.clear();
  fADCTime = 1;
  fTDCTime = -1;
  fPGACX = std::numeric_limits<double>::quiet_NaN();
}

// ============== Print ==============
// purpose: Print a compact EMMA event summary.
// inputs: ROOT option string
// outputs: none
void Emma::Print(Option_t *opt) const {
  printf("Emma summary\n");
  printf("\tadc:    %lu\n", fADC.size());
  printf("\ttdc:    %lu\n", fTDC.size());
  printf("\tsi:     %lu\n", fSi.size());
  printf("\tic1:    %lu\n", fIC1.size());
  printf("\tic2:    %lu\n", fIC2.size());
  printf("\tic3:    %lu\n", fIC3.size());
  printf("\tic4:    %lu\n", fIC4.size());
  printf("\tanodes: %lu\n", fAnodes.size());
  printf("\tleft:   %lu\n", fLeft.size());
  printf("\tright:  %lu\n", fRight.size());
  printf("\ttop:    %lu\n", fTop.size());
  printf("\tbot:    %lu\n", fBot.size());
  printf("\tpgacx:  %.3f\n", fPGACX);
}

// ============== AddADC ==============
// purpose: Store one reduced EMMA ADC hit.
// inputs: source fragment
// outputs: none
void Emma::AddADC(const Fragment& frag) {
  fADC.emplace_back(frag);
}

// ============== AddTDC ==============
// purpose: Store one reduced EMMA TDC hit.
// inputs: source fragment
// outputs: none
void Emma::AddTDC(const Fragment& frag) {
  fTDC.emplace_back(frag);
}

// ============== BuildHits ==============
// purpose: Sort reduced ADC/TDC hits into detector groups and calculate derived values.
// inputs: none
// outputs: none
void Emma::BuildHits() {

  fSi.clear();
  fIC1.clear();
  fIC2.clear();
  fIC3.clear();
  fIC4.clear();
  fAnodes.clear();
  fLeft.clear();
  fRight.clear();
  fTop.clear();
  fBot.clear();

  if(fADC.size()) { fADCTime = fADC.front().TimestampNs(); }
  if(fTDC.size()) { fTDCTime = fTDC.front().TimestampNs(); }

  for(const auto& hit : fADC) {
    int channel = hit.Address() & 0xff;
    if(channel == 3) {
      fSi.push_back(hit);
    } else if(channel == 16) {
      fIC1.push_back(hit);
    } else if(channel == 17) {
      fIC2.push_back(hit);
    } else if(channel == 18) {
      fIC3.push_back(hit);
    } else if(channel == 19) {
      fIC4.push_back(hit);
    }
  }

  for(const auto& hit : fTDC) {
    int channel = hit.Address() & 0xff;
    if(channel >= 0 && channel <= 2) {
      fAnodes.push_back(hit);
    } else if(channel == 3) {
      fLeft.push_back(hit);
    } else if(channel == 4) {
      fRight.push_back(hit);
    } else if(channel == 5) {
      fTop.push_back(hit);
    } else if(channel == 6) {
      fBot.push_back(hit);
    }
  }

  fPGACX = CalculatePGACX();

}

// ============== CalculatePGACX ==============
// purpose: Calculate PGAC X position from anode, left, and right charges.
// inputs: none
// outputs: PGAC X position, or NaN when inputs are incomplete
double Emma::CalculatePGACX() const {
  const double fLdelay = 40;
  const double fRdelay = 20;
  const double fXlength = 80.;
  const double invalid = std::numeric_limits<double>::quiet_NaN();

  if((fLeft.empty() && fRight.empty()) || fAnodes.empty()) return invalid;

  double left = 0;
  double right = 0;
  std::vector<double> anodes;

  for(const auto& hit : fAnodes) {
    anodes.push_back(hit.Charge());
  }

  double anode = *std::min_element(anodes.begin(), anodes.end());

  for(const auto& hit : fLeft) {
    left = hit.Charge();
  }

  for(const auto& hit : fRight) {
    right = hit.Charge();
  }

  if(!fLeft.empty()) left -= anode;
  if(!fRight.empty()) right -= anode;

  double xsum = left + right;
  if(xsum == 0) return invalid;

  double xdiff = (right + fLdelay) - (left + fRdelay);
  return (xdiff / xsum) * fXlength;
}
