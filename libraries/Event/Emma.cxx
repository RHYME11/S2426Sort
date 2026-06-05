#include <Emma.h>

#include <algorithm>
#include <cstdio>
#include <limits>

ClassImp(Emma)

namespace {

// ============== PrintFragments ==============
// purpose: Print fragment name, energy, charge, and time for one detector group.
// inputs: group label and fragments
// outputs: none
void PrintFragments(const char* label, const std::vector<Fragment>& fragments) {
  printf("\t%s: %lu\n", label, fragments.size());
  for(const auto& frag : fragments) {
    printf("\t\t%s charge %.3f timestamp: %ldns\n",
           frag.Name().c_str(), frag.Charge(), frag.TimestampNs());
  }
}

}

Emma::Emma() { }

Emma::~Emma() { }

// ============== Set ==============
// purpose: Copy EMMA fragments from an event.
// inputs: source event
// outputs: none
void Emma::Set(const Event& event) {
  Clear();

  for(size_t index : event.Si()) {
    fSi.push_back(event.FragmentAt(index));
  }

  for(const auto& group : event.ICs()) {
    int ic = group.first;
    for(size_t index : group.second) {
      fICs[ic].push_back(event.FragmentAt(index));
    }
  }

  for(size_t index : event.Anodes()) {
    fAnodes.push_back(event.FragmentAt(index));
  }

  for(size_t index : event.Left()) {
    fLeft.push_back(event.FragmentAt(index));
  }

  for(size_t index : event.Right()) {
    fRight.push_back(event.FragmentAt(index));
  }

  for(size_t index : event.Top()) {
    fTop.push_back(event.FragmentAt(index));
  }

  for(size_t index : event.Bot()) {
    fBot.push_back(event.FragmentAt(index));
  }

  SetPGACX(CalculatePGACX());
}

// ============== Clear ==============
// purpose: Clear stored EMMA fragments.
// inputs: none
// outputs: none
void Emma::Clear() {
  fSi.clear();
  fICs.clear();
  fAnodes.clear();
  fLeft.clear();
  fRight.clear();
  fTop.clear();
  fBot.clear();
  fPGACX = std::numeric_limits<double>::quiet_NaN();
}

// ============== Print ==============
// purpose: Print an EMMA summary and PGAC X position.
// inputs: none
// outputs: none
void Emma::Print() const {
  printf("Emma summary\n");
  PrintFragments("si", fSi);

  printf("\tic groups: %lu\n", fICs.size());
  for(const auto& group : fICs) {
    printf("\tic %i\n", group.first);
    PrintFragments("fragments", group.second);
  }

  PrintFragments("anodes", fAnodes);
  PrintFragments("left", fLeft);
  PrintFragments("right", fRight);
  PrintFragments("top", fTop);
  PrintFragments("bot", fBot);
  printf("\tPGACX: %.3f\n", PGACX());
}

// ============== CalculatePGACX ==============
// purpose: Calculate PGAC X position from left, right, and anode charges.
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

  for(const auto& frag : fAnodes) {
    anodes.push_back(frag.Charge());
  }

  double anode = *std::min_element(anodes.begin(), anodes.end());

  for(const auto& frag : fLeft) {
    left = frag.Charge();
  }

  for(const auto& frag : fRight) {
    right = frag.Charge();
  }

  if(!fLeft.empty()) left -= anode;
  if(!fRight.empty()) right -= anode;

  double xsum = left + right;
  if(xsum == 0) return invalid;

  //double xdiff = (left + fLdelay) - (right + fRdelay);
  double xdiff = (right + fLdelay) - (left + fRdelay);
  return (xdiff / xsum) * fXlength;
}
