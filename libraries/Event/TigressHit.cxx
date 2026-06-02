#include <TigressHit.h>

#include <cstdio>

ClassImp(TigressHit)

namespace {

// ============== PrintFragments ==============
// purpose: Print fragment name, energy, charge, and time for one detector group.
// inputs: group label and fragments
// outputs: none
void PrintFragments(const char* label, const std::vector<Fragment>& fragments) {
  printf("\t%s: %lu\n", label, fragments.size());
  for(const auto& frag : fragments) {
    printf("\t\t%s energy: %.3f charge: %.3f time: %.3fns\n",
           frag.Name().c_str(), frag.Energy(), frag.Charge(), frag.Time());
  }
}

}

TigressHit::TigressHit() { }

TigressHit::~TigressHit() { }

// ============== Clear ==============
// purpose: Clear stored TIGRESS fragments and array number.
// inputs: none
// outputs: none
void TigressHit::Clear() {
  fArryNumber = -1;
  fCores.clear();
  fSegments.clear();
  fBgos.clear();
}

// ============== Print ==============
// purpose: Print array number and stored fragment name, energy, charge, and time.
// inputs: none
// outputs: none
void TigressHit::Print() const {
  printf("TigressHit array number: %i\n", fArryNumber);
  PrintFragments("cores", fCores);
  PrintFragments("segments", fSegments);
  PrintFragments("bgo", fBgos);
}
