#include <Emma.h>

#include <algorithm>
#include <cstdio>
#include <limits>
#include <set>

ClassImp(EmmaHit)
ClassImp(Emma)

// ============== EmmaHit ==============
// Purpose: Copy the stored EMMA quantities from one fragment.
// Inputs: Source fragment.
// Outputs: Initialized EMMA hit.
EmmaHit::EmmaHit(const Fragment& frag) {
  fAddress = frag.Address();
  fTimestamp = frag.Timestamp();
  fTimestampNs = frag.TimestampNs();
  fTime = frag.Time();
  fCharge = frag.Charge();
  fEnergy = frag.Energy();
  fName = frag.Name();
  fNumber = frag.Number();
}

// ============== Clear ==============
// Purpose: Reset all stored EMMA hit quantities.
// Inputs: ROOT option string.
// Outputs: None.
void EmmaHit::Clear(Option_t *opt) {
  fAddress = -1;
  fTimestamp = -1;
  fTimestampNs = -1;
  fTime = -1;
  fCharge = -1;
  fEnergy = -1;
  fName.clear();
  fNumber = -1;
}

// ============== Print ==============
// Purpose: Print all quantities stored in one EMMA hit.
// Inputs: ROOT option string.
// Outputs: EMMA hit summary written to stdout.
void EmmaHit::Print(Option_t *opt) const {
  printf("EmmaHit @ 0x%016lx\n", fTimestamp);
  printf("\tname:        %s\n", fName.c_str());
  printf("\taddress:     0x%08x\n", fAddress);
  printf("\tnumber:      %i\n", fNumber);
  printf("\ttimestamp:   %ld\n", fTimestamp);
  printf("\ttimestampNs: %ld\n", fTimestampNs);
  printf("\ttime:        %.1f\n", fTime);
  printf("\tcharge:      %.2f\n", fCharge);
  printf("\tenergy:      %.1f\n", fEnergy);
}

// ============== Clear ==============
// Purpose: Clear EMMA hit groups and the cached PGAC X position.
// Inputs: ROOT option string.
// Outputs: None.
void Emma::Clear(Option_t *opt) {
  fSi.clear();
  fIC0.clear();
  fIC1.clear();
  fIC2.clear();
  fIC3.clear();
  fAnode.clear();
  fLeft.clear();
  fRight.clear();
  fTop.clear();
  fBot.clear();
  fPGACX = std::numeric_limits<double>::quiet_NaN();
}

// ============== Print ==============
// Purpose: Print the cached PGAC X position and all EMMA group sizes.
// Inputs: ROOT option string.
// Outputs: EMMA event summary written to stdout.
void Emma::Print(Option_t *opt) const {
  printf("Emma\n");
  printf("\tPGACX: %.3f\n", fPGACX);
  printf("\tSi:    %lu\n", fSi.size());
  printf("\tIC0:   %lu\n", fIC0.size());
  printf("\tIC1:   %lu\n", fIC1.size());
  printf("\tIC2:   %lu\n", fIC2.size());
  printf("\tIC3:   %lu\n", fIC3.size());
  printf("\tAnode: %lu\n", fAnode.size());
  printf("\tLeft:  %lu\n", fLeft.size());
  printf("\tRight: %lu\n", fRight.size());
  printf("\tTop:   %lu\n", fTop.size());
  printf("\tBot:   %lu\n", fBot.size());
}

// ============== BuildHits ==============
// Purpose: Group EMMA fragments and keep the first EMMT hit per address.
// Inputs: Source detector event.
// Outputs: Rebuilt EMMA groups and cached PGAC X position.
void Emma::BuildHits(const DetectorEvent& event) {
  Clear();

  std::set<int> emmtAddresses;
  for(const auto& frag : event.Fragments()) {
    const int channel = frag.Address() & 0xff;

    if(frag.DetType() == 13) {
      if(channel == 3) {
        fSi.emplace_back(frag);
      } else if(channel == 16) {
        fIC0.emplace_back(frag);
      } else if(channel == 17) {
        fIC1.emplace_back(frag);
      } else if(channel == 18) {
        fIC2.emplace_back(frag);
      } else if(channel == 19) {
        fIC3.emplace_back(frag);
      }
      continue;
    }

    if(frag.DetType() != 14 || !emmtAddresses.emplace(frag.Address()).second) {
      continue;
    }

    if(channel >= 0 && channel <= 2) {
      fAnode.emplace_back(frag);
    } else if(channel == 3) {
      fLeft.emplace_back(frag);
    } else if(channel == 4) {
      fRight.emplace_back(frag);
    } else if(channel == 5) {
      fTop.emplace_back(frag);
    } else if(channel == 6) {
      fBot.emplace_back(frag);
    }
  }

  fPGACX = CalculatePGACX();
}

// ============== CalculatePGACX ==============
// Purpose: Calculate PGAC X from the stored anode, left, and right charges.
// Inputs: None.
// Outputs: PGAC X position, or NaN when inputs are incomplete.
double Emma::CalculatePGACX() const {
  const double fLdelay = 40;
  const double fRdelay = 20;
  const double fXlength = 80.;
  const double invalid = std::numeric_limits<double>::quiet_NaN();

  if((fLeft.empty() && fRight.empty()) || fAnode.empty()) return invalid;

  double left = 0;
  double right = 0;
  std::vector<double> anodes;
  anodes.reserve(fAnode.size());

  for(const auto& hit : fAnode) {
    anodes.push_back(hit.Charge());
  }

  const double anode = *std::min_element(anodes.begin(), anodes.end());

  for(const auto& hit : fLeft) {
    left = hit.Charge();
  }

  for(const auto& hit : fRight) {
    right = hit.Charge();
  }

  if(!fLeft.empty()) left -= anode;
  if(!fRight.empty()) right -= anode;

  const double xsum = left + right;
  if(xsum == 0) return invalid;

  const double xdiff = (right + fLdelay) - (left + fRdelay);
  return (xdiff / xsum) * fXlength;
}
