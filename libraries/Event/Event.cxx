#include <Event.h>

#include <cstdio>

ClassImp(Event)

Event::Event() { }

Event::Event(const std::vector<Fragment>& fragments) {
  Set(fragments);
}

Event::~Event() { }

// ============== Set ==============
// purpose: Copy event fragments and rebuild detector-category indices.
// inputs: vector of fragments
// outputs: none
void Event::Set(const std::vector<Fragment>& fragments) {
  Clear();
  fFragments = fragments;

  bool hasIC = false;

  for(size_t i = 0; i < fFragments.size(); ++i) {
    const Fragment& frag = fFragments.at(i);
    int c = frag.Address() & 0xff;

    switch(frag.DetType()) {
      case 0:
      case 1:
        fCores.push_back(i);
        break;
      case 2:
        fSegments.push_back(i);
        break;
      case 3:
        fBgos.push_back(i);
        break;
      case 13:
        if(c == 3) {
          fSi.push_back(i);
        } else if(c >= 16 && c <= 19) {
          fICs[c - 16].push_back(i);
          hasIC = true;
        }
        break;
      case 14:
        if(c >= 0 && c <= 2) {
          fAnodes.push_back(i);
        } else if(c == 3) {
          fLeft.push_back(i);
        } else if(c == 4) {
          fRight.push_back(i);
        } else if(c == 5) {
          fPGACTop.push_back(i);
        } else if(c == 6) {
          fPGACBot.push_back(i);
        }
        break;
      default:
        break;
    }
  }

  fGood = !fCores.empty()
       && !fSi.empty()
       && !fLeft.empty()
       && !fRight.empty()
       && !fAnodes.empty()
       && hasIC;
}

// ============== Copy ==============
// purpose: Copy every stored field from an input Event into a new Event object.
// inputs: source Event
// outputs: copied Event
Event Event::Copy(const Event& event) {
  Event copy;

  copy.fFragments = event.fFragments;
  copy.fCores = event.fCores;
  copy.fSegments = event.fSegments;
  copy.fBgos = event.fBgos;
  copy.fSi = event.fSi;
  copy.fICs = event.fICs;
  copy.fAnodes = event.fAnodes;
  copy.fLeft = event.fLeft;
  copy.fRight = event.fRight;
  copy.fPGACTop = event.fPGACTop;
  copy.fPGACBot = event.fPGACBot;
  copy.fGood = event.fGood;

  return copy;
}

// ============== Print ==============
// purpose: Print a concise event summary.
// inputs: none
// outputs: none
void Event::Print() const {
  printf("Event summary\n");
  printf("\tfragments: %lu\n", fFragments.size());
  printf("\tgood:      %s\n", fGood ? "true" : "false");
  printf("\tcores:     %lu\n", fCores.size());
  printf("\tsegments:  %lu\n", fSegments.size());
  printf("\tbgo:       %lu\n", fBgos.size());
  printf("\tsi:        %lu\n", fSi.size());
  printf("\tic groups: %lu\n", fICs.size());
  printf("\tanodes:    %lu\n", fAnodes.size());
  printf("\tleft:      %lu\n", fLeft.size());
  printf("\tright:     %lu\n", fRight.size());
  printf("\tpgac top:  %lu\n", fPGACTop.size());
  printf("\tpgac bot:  %lu\n", fPGACBot.size());
}

// ============== Clear ==============
// purpose: Clear stored fragments, category indices, and event status.
// inputs: none
// outputs: none
void Event::Clear() {
  fFragments.clear();

  fCores.clear();
  fSegments.clear();
  fBgos.clear();

  fSi.clear();
  fICs.clear();

  fAnodes.clear();
  fLeft.clear();
  fRight.clear();
  fPGACTop.clear();
  fPGACBot.clear();

  fGood = false;
}
