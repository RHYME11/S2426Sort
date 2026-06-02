#include <OutputManager.h>

#include <cstdio>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

OutputManager *OutputManager::fOutputManager = 0;

OutputManager::OutputManager() { }

OutputManager::~OutputManager() {
  CloseFile(fFragmentFile, fFragmentTree, fFragmentFilename);
  CloseFile(fEventFile, fEventTree, fEventFilename);
  CloseFile(fGoodEventFile, fGoodEventTree, fGoodEventFilename);
  CloseFile(fAnalysisFile, fAnalysisTree, fAnalysisFilename);
}

OutputManager *OutputManager::Get() {
  if(!fOutputManager)
    fOutputManager = new OutputManager;
  return fOutputManager;
}

// ============== Open ==============
// purpose: Open ROOT output files and create output trees.
// inputs: run number, subrun number, and output mode
// outputs: none
void OutputManager::Open(int run, int subrun, OutputMode mode) {
  switch(mode) {
    case OutputMode::Sort:
      OpenSort(run, subrun);
      break;
    case OutputMode::Analysis:
      OpenAnalysis(run, subrun);
      break;
  }
}

// ============== OpenSort ==============
// purpose: Open fragment, event, and good-event output files.
// inputs: run number and subrun number
// outputs: none
void OutputManager::OpenSort(int run, int subrun) {
  fFragmentFilename = Form("Fragment%i_%03i.root", run, subrun);
  fEventFilename = Form("event%i_%03i.root", run, subrun);
  fGoodEventFilename = Form("goodevent%i_%03i.root", run, subrun);

  fFragmentFile = new TFile(fFragmentFilename.c_str(), "recreate");
  fFragmentTree = new TTree("FragmentTree", "FragmentTree");
  fFragmentTree->Branch("fragment", "Fragment", &fFragmentBranch, 32000, 0);

  fEventFile = new TFile(fEventFilename.c_str(), "recreate");
  fEventTree = new TTree("eventTree", "eventTree");
  fEventTree->Branch("event", "Event", &fEventBranch, 32000, 0);

  fGoodEventFile = new TFile(fGoodEventFilename.c_str(), "recreate");
  fGoodEventTree = new TTree("eventTree", "eventTree");
  fGoodEventTree->Branch("event", "Event", &fGoodEventBranch, 32000, 0);
}

// ============== OpenAnalysis ==============
// purpose: Open analysis output file and tree.
// inputs: run number and subrun number
// outputs: none
void OutputManager::OpenAnalysis(int run, int subrun) {
  fAnalysisFilename = Form("Analysis%i_%03i.root", run, subrun);

  fAnalysisFile = new TFile(fAnalysisFilename.c_str(), "recreate");
  fAnalysisTree = new TTree("AnalysisTree", "AnalysisTree");
  fAnalysisTree->Branch("fTigress", "Tigress", &fTigressBranch, 32000, 0);
  fAnalysisTree->Branch("fEmma", "Emma", &fEmmaBranch, 32000, 0);
  fAnalysisTree->Branch("fEvent", "Event", &fAnalysisEventBranch, 32000, 0);
}

// ============== Close ==============
// purpose: Write non-empty trees and close all output files.
// inputs: none
// outputs: none
void OutputManager::Close() {
  if(fOutputManager)
    delete fOutputManager;
  fOutputManager = 0;
}

// ============== FillFragment ==============
// purpose: Fill one sorted fragment into fragment tree.
// inputs: fragment
// outputs: none
void OutputManager::FillFragment(const Fragment& fragment) {
  if(!fFragmentTree) return;
  fFragment = fragment;
  fFragmentTree->Fill();
}

// ============== FillEvent ==============
// purpose: Fill one built event into event tree.
// inputs: event
// outputs: none
void OutputManager::FillEvent(const Event& event) {
  if(!fEventTree) return;
  fEvent = event;
  fEventTree->Fill();
}

// ============== FillGoodEvent ==============
// purpose: Fill one good event into good-event tree.
// inputs: event
// outputs: none
void OutputManager::FillGoodEvent(const Event& event) {
  if(!fGoodEventTree) return;
  fGoodEvent = event;
  fGoodEventTree->Fill();
}

// ============== FillAnalysis ==============
// purpose: Fill one analysis event into analysis tree.
// inputs: TIGRESS object, EMMA object, and remaining event fragments
// outputs: none
void OutputManager::FillAnalysis(const Tigress& tigress, const Emma& emma, const Event& event) {
  if(!fAnalysisTree) return;
  fTigress = tigress;
  fEmma = emma;
  fAnalysisEvent = event;
  fAnalysisTree->Fill();
}

// ============== CloseFile ==============
// purpose: Write a non-empty tree or remove its empty output file.
// inputs: ROOT file, ROOT tree, and filename
// outputs: none
void OutputManager::CloseFile(TFile*& file, TTree*& tree, const std::string& filename) {
  if(!file) return;

  file->cd();
  if(tree && tree->GetEntries() > 0) {
    tree->Write();
    file->Close();
  } else {
    file->Close();
    if(!filename.empty()) std::remove(filename.c_str());
  }

  delete file;
  file = nullptr;
  tree = nullptr;
}
