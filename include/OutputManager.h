#ifndef __OUTPUTMANAGER_H__
#define __OUTPUTMANAGER_H__

#include <string>

#include <EventProcess.h>

class TFile;
class TTree;

class OutputManager {
  private:
    OutputManager();
    static OutputManager *fOutputManager;

  public:
    virtual ~OutputManager();
    static OutputManager *Get();

    // ============== Open ==============
    // purpose: Open event and fragment ROOT files and create their trees.
    // inputs: run and subrun numbers
    // outputs: none
    void Open(int run, int subrun);

    // ============== FillEvent ==============
    // purpose: Fill one detector event into the selected event tree.
    // inputs: detector event
    // outputs: none
    void FillEvent(const DetectorEvent& event);

    // ============== FillFragment ==============
    // purpose: Fill one time-ordered fragment into the fragment tree.
    // inputs: fragment
    // outputs: none
    void FillFragment(const Fragment& fragment);

    // ============== Close ==============
    // purpose: Write all trees and close both ROOT output files.
    // inputs: none
    // outputs: none
    static void Close();

  private:
    std::string fEventFilename;
    std::string fFragmentFilename;

    TFile *fEventFile{nullptr};
    TFile *fFragmentFile{nullptr};
    TTree *fPromptTree{nullptr};
    TTree *fBgTree{nullptr};
    TTree *fFragmentTree{nullptr};

    DetectorEvent fEvent;
    DetectorEvent *fEventBranch{&fEvent};

    Fragment fFragment;
    Fragment *fFragmentBranch{&fFragment};
};

#endif
