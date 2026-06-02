#ifndef __OUTPUTMANAGER_H__
#define __OUTPUTMANAGER_H__

#include <string>

#include <Event.h>
#include <Fragment.h>
#include <TEmma.h>
#include <Tigress.h>

enum class OutputMode {
  Sort,
  Analysis
};

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
    // purpose: Open ROOT output files and create output trees.
    // inputs: run number, subrun number, and output mode
    // outputs: none
    void Open(int run, int subrun, OutputMode mode = OutputMode::Sort);

    // ============== Close ==============
    // purpose: Write non-empty trees and close all output files.
    // inputs: none
    // outputs: none
    static void Close();

    // ============== FillFragment ==============
    // purpose: Fill one sorted fragment into fragment tree.
    // inputs: fragment
    // outputs: none
    void FillFragment(const Fragment& fragment);

    // ============== FillEvent ==============
    // purpose: Fill one built event into event tree.
    // inputs: event
    // outputs: none
    void FillEvent(const Event& event);

    // ============== FillGoodEvent ==============
    // purpose: Fill one good event into good-event tree.
    // inputs: event
    // outputs: none
    void FillGoodEvent(const Event& event);

    // ============== FillAnalysis ==============
    // purpose: Fill one analysis event into analysis tree.
    // inputs: TIGRESS and EMMA detector objects
    // outputs: none
    void FillAnalysis(const Tigress& tigress, const TEmma& emma);

  private:
    void OpenSort(int run, int subrun);
    void OpenAnalysis(int run, int subrun);
    void CloseFile(TFile*& file, TTree*& tree, const std::string& filename);

    std::string fFragmentFilename;
    std::string fEventFilename;
    std::string fGoodEventFilename;
    std::string fAnalysisFilename;

    TFile *fFragmentFile{nullptr};
    TFile *fEventFile{nullptr};
    TFile *fGoodEventFile{nullptr};
    TFile *fAnalysisFile{nullptr};

    TTree *fFragmentTree{nullptr};
    TTree *fEventTree{nullptr};
    TTree *fGoodEventTree{nullptr};
    TTree *fAnalysisTree{nullptr};

    Fragment fFragment;
    Event fEvent;
    Event fGoodEvent;
    Tigress fTigress;
    TEmma fEmma;

    Fragment *fFragmentBranch{&fFragment};
    Event *fEventBranch{&fEvent};
    Event *fGoodEventBranch{&fGoodEvent};
    Tigress *fTigressBranch{&fTigress};
    TEmma *fEmmaBranch{&fEmma};
};

#endif
