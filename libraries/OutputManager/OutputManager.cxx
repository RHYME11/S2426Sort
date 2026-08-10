#include <OutputManager.h>

#include <cstdio>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

OutputManager *OutputManager::fOutputManager = 0;

OutputManager::OutputManager() { }

OutputManager::~OutputManager() {
  if(fEventFile) {
    fEventFile->cd();
    if(fPromptTree) fPromptTree->Write();
    if(fBgTree) fBgTree->Write();
    fEventFile->Close();

    delete fEventFile;
    fEventFile = nullptr;
    fPromptTree = nullptr;
    fBgTree = nullptr;
  }

  if(fFragmentFile) {
    fFragmentFile->cd();
    if(fFragmentTree) fFragmentTree->Write();
    fFragmentFile->Close();

    delete fFragmentFile;
    fFragmentFile = nullptr;
    fFragmentTree = nullptr;
  }
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
// purpose: Open event and fragment ROOT files and create their trees.
// inputs: run and subrun numbers
// outputs: none
void OutputManager::Open(int run, int subrun) {
  if(fEventFile || fFragmentFile) return;

  fEventFilename = Form("event%i_%03i.root", run, subrun);
  fEventFile = TFile::Open(fEventFilename.c_str(), "recreate");
  if(!fEventFile || fEventFile->IsZombie()) {
    printf("Failed to open event output file %s\n", fEventFilename.c_str());
    delete fEventFile;
    fEventFile = nullptr;
  } else {
    fEventFile->cd();

    fPromptTree = new TTree("PromptTree", "Prompt events");
    fPromptTree->Branch("event", "DetectorEvent", &fEventBranch, 32000, 0);

    fBgTree = new TTree("BgTree", "Background events");
    fBgTree->Branch("event", "DetectorEvent", &fEventBranch, 32000, 0);
  }

  fFragmentFilename = Form("fragment%i_%03i.root", run, subrun);
  fFragmentFile = TFile::Open(fFragmentFilename.c_str(), "recreate");
  if(!fFragmentFile || fFragmentFile->IsZombie()) {
    printf("Failed to open fragment output file %s\n", fFragmentFilename.c_str());
    delete fFragmentFile;
    fFragmentFile = nullptr;
  } else {
    fFragmentFile->cd();
    fFragmentTree = new TTree("FragmentTree", "Time-ordered fragments");
    fFragmentTree->Branch("Fragment", "Fragment", &fFragmentBranch, 32000, 0);
  }
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

// ============== FillFragment ==============
// purpose: Fill one time-ordered fragment into the fragment tree.
// inputs: fragment
// outputs: none
void OutputManager::FillFragment(const Fragment& fragment) {
  if(!fFragmentTree) return;

  fFragment = fragment;
  fFragmentTree->Fill();
}

// ============== Close ==============
// purpose: Write all trees and close both ROOT output files.
// inputs: none
// outputs: none
void OutputManager::Close() {
  if(fOutputManager)
    delete fOutputManager;
  fOutputManager = 0;
}
