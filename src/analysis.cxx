#include <cstdio>

#include <TFile.h>
#include <TTree.h>

#include <Channel.h>
#include <Event.h>
#include <OutputManager.h>
#include <TEmma.h>
#include <Tigress.h>
#include <utils.h>

// ============== main ==============
// purpose: Convert good-event Event entries into TIGRESS and EMMA analysis objects.
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
  TEmma emma;

  Long64_t entries = inputTree->GetEntries();
  Long64_t i = 0;
  for(; i < entries; ++i) {
    inputTree->GetEntry(i);
    if(!event) continue;

    tigress.Set(*event);
    emma.Set(*event);

    OutputManager::Get()->FillAnalysis(tigress, emma);

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
