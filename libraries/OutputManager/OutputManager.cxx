#include <OutputManager.h>

#include <cstdio>

#include <TFile.h>
#include <TString.h>
#include <TTree.h>

OutputManager *OutputManager::fOutputManager = 0;

OutputManager::OutputManager() { }

OutputManager::~OutputManager() {
  CloseFile(fListFile, fListTree, fListFilename);
  CloseFile(fEventFile, fEventTree, fEventFilename);
  CloseFile(fGoodEventFile, fGoodEventTree, fGoodEventFilename);
}

OutputManager *OutputManager::Get() {
  if(!fOutputManager)
    fOutputManager = new OutputManager;
  return fOutputManager;
}

// ============== Open ==============
// purpose: Open ROOT output files and create output trees.
// inputs: run number and subrun number
// outputs: none
void OutputManager::Open(int run, int subrun) {
  fListFilename = Form("list%i_%03i.root", run, subrun);
  fEventFilename = Form("event%i_%03i.root", run, subrun);
  fGoodEventFilename = Form("goodevent%i_%03i.root", run, subrun);

  fListFile = new TFile(fListFilename.c_str(), "recreate");
  fListTree = new TTree("listTree", "listTree");
  fListTree->Branch("fragment", "Fragment", &fFragmentPtr, 32000, 0);

  fEventFile = new TFile(fEventFilename.c_str(), "recreate");
  fEventTree = new TTree("eventTree", "eventTree");
  fEventTree->Branch("event", "Event", &fEventPtr, 32000, 0);

  fGoodEventFile = new TFile(fGoodEventFilename.c_str(), "recreate");
  fGoodEventTree = new TTree("eventTree", "eventTree");
  fGoodEventTree->Branch("event", "Event", &fGoodEventPtr, 32000, 0);
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
// purpose: Fill one sorted fragment into list tree.
// inputs: fragment
// outputs: none
void OutputManager::FillFragment(const Fragment& fragment) {
  if(!fListTree) return;
  fFragment = fragment;
  fListTree->Fill();
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
