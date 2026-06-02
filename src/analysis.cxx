#include <cstdio>
#include <vector>

#include <TFile.h>
#include <TTree.h>

#include <Channel.h>
#include <Emma.h>
#include <Event.h>
#include <OutputManager.h>
#include <Tigress.h>
#include <utils.h>

// ============== MarkUsedFragments ==============
// purpose: Mark fragment indices that were copied into detector-level objects.
// inputs: used-flag vector and fragment indices
// outputs: none
void MarkUsedFragments(std::vector<bool>& used, const std::vector<size_t>& indices) {
  for(size_t index : indices) {
    if(index < used.size()) used[index] = true;
  }
}

// ============== MakeRemainingEvent ==============
// purpose: Build an Event from non-detector fragments, excluding dummy channels.
// inputs: source good event
// outputs: event containing remaining non-dummy fragments
Event MakeRemainingEvent(const Event& event) {
  std::vector<bool> used(event.Size(), false);
  MarkUsedFragments(used, event.Cores());
  MarkUsedFragments(used, event.Segments());
  MarkUsedFragments(used, event.Bgos());
  MarkUsedFragments(used, event.Si());
  MarkUsedFragments(used, event.Anodes());
  MarkUsedFragments(used, event.Left());
  MarkUsedFragments(used, event.Right());
  MarkUsedFragments(used, event.Top());
  MarkUsedFragments(used, event.Bot());

  for(const auto& group : event.ICs()) {
    MarkUsedFragments(used, group.second);
  }

  std::vector<Fragment> fragments;
  for(size_t i = 0; i < event.Size(); ++i) {
    const Fragment& frag = event.FragmentAt(i);
    if(!used[i] && frag.Name() != "dummy") fragments.push_back(frag);
  }

  return Event(fragments);
}

// ============== main ==============
// purpose: Convert good-event Event entries into TIGRESS, EMMA, and remaining analysis objects.
// inputs: good-event ROOT file path
// outputs: Analysis ROOT file
int main(int argc, char **argv) {
  if(argc < 2) {
    printf("usage: %s goodevent<run>_<subrun>.root\n", argv[0]);
    return 1;
  }

  int run = -1;
  int subrun = -1;
  getRunNumber(argv[1], run, subrun);
  if(run < 0 || subrun < 0) {
    printf("Could not parse run and subrun from %s\n", argv[1]);
    return 1;
  }

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");

  TFile *inputFile = TFile::Open(argv[1], "read");
  if(!inputFile || inputFile->IsZombie()) {
    printf("Could not open input file %s\n", argv[1]);
    return 1;
  }

  TTree *inputTree = static_cast<TTree*>(inputFile->Get("eventTree"));
  if(!inputTree) {
    printf("Could not find eventTree in %s\n", argv[1]);
    inputFile->Close();
    return 1;
  }

  Event *event = nullptr;
  inputTree->SetBranchAddress("event", &event);

  OutputManager::Get()->Open(run, subrun, OutputMode::Analysis);

  Tigress tigress;
  Emma emma;
  Event remaining;

  Long64_t entries = inputTree->GetEntries();
  Long64_t i = 0;
  for(; i < entries; ++i) {
    inputTree->GetEntry(i);
    if(!event) continue;

    tigress.Set(*event);
    emma.Set(*event);
    remaining = MakeRemainingEvent(*event);

    OutputManager::Get()->FillAnalysis(tigress, emma, remaining);

    if((i%2000) == 0){
      printf("run%i_%03i: on entry %lld / %lld\r", run, subrun, static_cast<long long>(i), static_cast<long long>(entries));
      fflush(stdout);
    }
  }

  OutputManager::Close();
  inputFile->Close();

  printf("run%i_%03i: on entry %lld / %lld\n", run, subrun, static_cast<long long>(i), static_cast<long long>(entries));
  return 0;
}
