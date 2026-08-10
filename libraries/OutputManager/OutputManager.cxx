#include <OutputManager.h>

#include <cstdio>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

OutputManager *OutputManager::fOutputManager = 0;

OutputManager::OutputManager() { }

OutputManager::~OutputManager() {
  if(!fEventFile) return;

  fEventFile->cd();
  if(fPromptTree) fPromptTree->Write();
  if(fBgTree) fBgTree->Write();
  fEventFile->Close();

  delete fEventFile;
  fEventFile = nullptr;
  fPromptTree = nullptr;
  fBgTree = nullptr;
}

// ============== Get ==============
// purpose: Return the process-wide output manager instance.
// inputs: none
// outputs: output manager pointer
OutputManager *OutputManager::Get() {
  if(!fOutputManager)
    fOutputManager = new OutputManager;
  return fOutputManager;
}

// ============== Open ==============
// purpose: Open the event ROOT file and create prompt and background trees.
// inputs: run and subrun numbers
// outputs: none
void OutputManager::Open(int run, int subrun) {
  if(fEventFile) return;

  fEventFilename = Form("event%i_%03i.root", run, subrun);
  fEventFile = TFile::Open(fEventFilename.c_str(), "recreate");
  if(!fEventFile || fEventFile->IsZombie()) {
    printf("Failed to open event output file %s\n", fEventFilename.c_str());
    delete fEventFile;
    fEventFile = nullptr;
    return;
  }

  fEventFile->cd();

  fPromptTree = new TTree("PromptTree", "Prompt events");
  fPromptTree->Branch("event", "DetectorEvent", &fEventBranch, 32000, 0);

  fBgTree = new TTree("BgTree", "Background events");
  fBgTree->Branch("event", "DetectorEvent", &fEventBranch, 32000, 0);
}

// ============== FillEvent ==============
// purpose: Fill one detector event into the selected event tree.
// inputs: detector event
// outputs: none
void OutputManager::FillEvent(const DetectorEvent& event) {
  if(!fEventFile) return;

  fEvent = event;

  if(event.Prompt()) {
    if(fPromptTree) fPromptTree->Fill();
  } else {
    if(fBgTree) fBgTree->Fill();
  }
}

// ============== Close ==============
// purpose: Write both event trees and close the event ROOT file.
// inputs: none
// outputs: none
void OutputManager::Close() {
  if(fOutputManager)
    delete fOutputManager;
  fOutputManager = 0;
}
