
#include <Emma.h>
#include <Fragment.h>



EmmaHit::EmmaHit(Fragment &frag) { }

void EmmaHit::Clear(Option_t *opt) {  }

void EmmaHit::Print(Option_t *opt) const {  }

void Emma::Clear(Option_t *opt) { 
  fADC.clear();
  fTDC.clear();
}

void Emma::Print(Option_t *opt) const {  }


void Emma::BuildHits() {

  if(fADC.size()) { fADCTime = fADC.front().Timestamp(); }
  if(fTDC.size()) { fTDCTime = fTDC.front().Timestamp(); }

}



