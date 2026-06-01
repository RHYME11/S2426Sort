#ifndef __OUTPUTMANAGER_H__
#define __OUTPUTMANAGER_H__

#include <string>

#include <Event.h>
#include <Fragment.h>

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
    // inputs: run number and subrun number
    // outputs: none
    void Open(int run, int subrun);

    // ============== Close ==============
    // purpose: Write non-empty trees and close all output files.
    // inputs: none
    // outputs: none
    static void Close();

    // ============== FillFragment ==============
    // purpose: Fill one sorted fragment into list tree.
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

  private:
    void CloseFile(TFile*& file, TTree*& tree, const std::string& filename);

    std::string fListFilename;
    std::string fEventFilename;
    std::string fGoodEventFilename;

    TFile *fListFile{nullptr};
    TFile *fEventFile{nullptr};
    TFile *fGoodEventFile{nullptr};

    TTree *fListTree{nullptr};
    TTree *fEventTree{nullptr};
    TTree *fGoodEventTree{nullptr};

    Fragment fFragment;
    Event fEvent;
    Event fGoodEvent;

    Fragment *fFragmentPtr{&fFragment};
    Event *fEventPtr{&fEvent};
    Event *fGoodEventPtr{&fGoodEvent};
};

#endif
