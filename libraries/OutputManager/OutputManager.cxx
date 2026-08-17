#include <OutputManager.h>

#include <cmath>
#include <cstdio>
#include <filesystem>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

OutputManager *OutputManager::fOutputManager = 0;

OutputManager::OutputManager() { }

OutputManager::~OutputManager() {
  std::lock_guard<std::mutex> lock(fMutex);

  if(fFragmentFile) {
    fFragmentFile->cd();
    if(fFragmentTree) fFragmentTree->Write();
    fFragmentFile->Close();
    delete fFragmentFile;
    fFragmentFile = nullptr;
    fFragmentTree = nullptr;
  }

  if(fEventFile) {
    fEventFile->cd();
    if(fEventTree) fEventTree->Write();
    fEventFile->Close();
    delete fEventFile;
    fEventFile = nullptr;
    fEventTree = nullptr;
  }

  if(fPhysicsFile) {
    fPhysicsFile->cd();
    if(fBgTree) fBgTree->Write();
    if(fPromptGoodTree) fPromptGoodTree->Write();
    if(fPromptBadTree) fPromptBadTree->Write();
    fPhysicsFile->Close();
    delete fPhysicsFile;
    fPhysicsFile = nullptr;
    fBgTree = nullptr;
    fPromptGoodTree = nullptr;
    fPromptBadTree = nullptr;
  }
}

// ============== Get ==============
// Purpose: Return the process-wide output manager instance.
// Inputs: None.
// Outputs: Output manager pointer.
OutputManager *OutputManager::Get() {
  if(!fOutputManager)
    fOutputManager = new OutputManager;
  return fOutputManager;
}

// ============== Open ==============
// Purpose: Open the three TTree ROOT files and create their trees.
// Inputs: Run and subrun numbers.
// Outputs: None.
void OutputManager::Open(int run, int subrun) {
  if(fFragmentFile || fEventFile || fPhysicsFile) return;

  const std::string outDir = "ttreeOutput";
  if(!std::filesystem::exists(outDir)) {
    std::filesystem::create_directory(outDir);
  }

  fFragmentFilename = Form("%s/fragment%i_%03i.root", outDir.c_str(), run, subrun);
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

  fEventFilename = Form("%s/event%i_%03i.root", outDir.c_str(), run, subrun);
  fEventFile = TFile::Open(fEventFilename.c_str(), "recreate");
  if(!fEventFile || fEventFile->IsZombie()) {
    printf("Failed to open event output file %s\n", fEventFilename.c_str());
    delete fEventFile;
    fEventFile = nullptr;
  } else {
    fEventFile->cd();
    fEventTree = new TTree("EventTree", "Built detector events");
    fEventTree->Branch("DetectorEvent", "DetectorEvent", &fEventBranch, 32000, 0);
  }

  fPhysicsFilename = Form("%s/physics%i_%03i.root", outDir.c_str(), run, subrun);
  fPhysicsFile = TFile::Open(fPhysicsFilename.c_str(), "recreate");
  if(!fPhysicsFile || fPhysicsFile->IsZombie()) {
    printf("Failed to open physics output file %s\n", fPhysicsFilename.c_str());
    delete fPhysicsFile;
    fPhysicsFile = nullptr;
  } else {
    fPhysicsFile->cd();

    fBgTree = new TTree("BgTree", "Background events");
    fBgTree->Branch("Emma", "Emma", &fEmmaBranch, 32000, 0);
    fBgTree->Branch("Tigress", "Tigress", &fTigressBranch, 32000, 0);

    fPromptGoodTree = new TTree("PromptGoodTree", "Prompt events with valid PGAC X");
    fPromptGoodTree->Branch("Emma", "Emma", &fEmmaBranch, 32000, 0);
    fPromptGoodTree->Branch("Tigress", "Tigress", &fTigressBranch, 32000, 0);

    fPromptBadTree = new TTree("PromptBadTree", "Prompt events without valid PGAC X");
    fPromptBadTree->Branch("Emma", "Emma", &fEmmaBranch, 32000, 0);
    fPromptBadTree->Branch("Tigress", "Tigress", &fTigressBranch, 32000, 0);
  }
}

// ============== FillEvent ==============
// Purpose: Fill one complete detector event into EventTree.
// Inputs: Detector event.
// Outputs: None.
void OutputManager::FillEvent(const DetectorEvent& event) {
  std::lock_guard<std::mutex> lock(fMutex);
  if(!fEventTree) return;

  fEvent = event;
  fEventTree->Fill();
}

// ============== FillPhysics ==============
// Purpose: Fill one Emma/Tigress pair into exactly one Physics tree.
// Inputs: Built Emma and Tigress objects.
// Outputs: None.
void OutputManager::FillPhysics(const Emma& emma, const Tigress& tigress) {
  std::lock_guard<std::mutex> lock(fMutex);
  if(!fPhysicsFile) return;

  fEmma = emma;
  fTigress = tigress;

  if(emma.Anode().empty()) {
    if(fBgTree) fBgTree->Fill();
  } else if(std::isfinite(emma.PGACX())) {
    if(fPromptGoodTree) fPromptGoodTree->Fill();
  } else {
    if(fPromptBadTree) fPromptBadTree->Fill();
  }
}

// ============== FillFragment ==============
// Purpose: Fill one time-ordered fragment before it leaves EventBuilder.
// Inputs: Fragment still owned by the EventBuilder queue.
// Outputs: None.
void OutputManager::FillFragment(const Fragment& fragment) {
  std::lock_guard<std::mutex> lock(fMutex);
  if(!fFragmentTree) return;

  fFragment = fragment;
  fFragmentTree->Fill();
}

// ============== Close ==============
// Purpose: Write all trees and close the three TTree ROOT files.
// Inputs: None.
// Outputs: None.
void OutputManager::Close() {
  if(fOutputManager)
    delete fOutputManager;
  fOutputManager = 0;
}
