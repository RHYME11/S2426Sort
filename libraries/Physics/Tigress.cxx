
#include <Tigress.h>
#include <Fragment.h>



TigressHit::TigressHit(Fragment &frag) { }

void TigressHit::Clear(Option_t *opt) {  }

void TigressHit::Print(Option_t *opt) const {  }



void Tigress::Clear(Option_t *opt) { 
  fCoreHits.clear();
  fSegmentHits.clear();
  fBGOHits.clear();

  fHits.clear(); 
}

void Tigress::Print(Option_t *opt) const {  }


void Tigress::BuildHits() {

  for(auto &frag : fCoreHits) {
    TigressHit hit(frag);
    fHits.emplace_back(hit);
  }


}



