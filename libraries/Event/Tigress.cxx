#include <Tigress.h>

#include <cstdio>
#include <map>

ClassImp(Tigress)

Tigress::Tigress() { }

Tigress::~Tigress() { }

// ============== Set ==============
// purpose: Build TIGRESS hits from event core, segment, and BGO fragments.
// inputs: source event
// outputs: none
void Tigress::Set(const Event& event) {
  Clear();

  std::map<int, TigressHit> hitsByArray;

  for(size_t index : event.Cores()) {
    const Fragment& frag = event.FragmentAt(index);
    int arryNumber = frag.ArryNumber();
    hitsByArray[arryNumber].SetArryNumber(arryNumber);
    hitsByArray[arryNumber].AddCore(frag);
  }

  for(size_t index : event.Segments()) {
    const Fragment& frag = event.FragmentAt(index);
    int arryNumber = frag.ArryNumber();
    hitsByArray[arryNumber].SetArryNumber(arryNumber);
    hitsByArray[arryNumber].AddSegment(frag);
  }

  for(size_t index : event.Bgos()) {
    const Fragment& frag = event.FragmentAt(index);
    int arryNumber = frag.ArryNumber();
    hitsByArray[arryNumber].SetArryNumber(arryNumber);
    hitsByArray[arryNumber].AddBgo(frag);
  }

  for(const auto& hit : hitsByArray) {
    // hit.second.SetBGOFired();
    fHits.push_back(hit.second);
  }
}

// ============== Clear ==============
// purpose: Clear stored TIGRESS hits.
// inputs: none
// outputs: none
void Tigress::Clear() {
  fHits.clear();
}

// ============== Print ==============
// purpose: Print a TIGRESS summary and all stored hits.
// inputs: none
// outputs: none
void Tigress::Print() const {
  printf("Tigress summary\n");
  printf("\thits: %lu\n", fHits.size());

  for(const auto& hit : fHits) {
    hit.Print();
  }
}
