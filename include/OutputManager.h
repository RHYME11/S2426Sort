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
    // purpose: Open the event ROOT file and create prompt and background trees.
    // inputs: run and subrun numbers
    // outputs: none
    void Open(int run, int subrun);

    // ============== FillEvent ==============
    // purpose: Fill one detector event into the selected event tree.
    // inputs: detector event
    // outputs: none
    void FillEvent(const DetectorEvent& event);

    // ============== Close ==============
    // purpose: Write both event trees and close the event ROOT file.
    // inputs: none
    // outputs: none
    static void Close();

  private:
    std::string fEventFilename;

    TFile *fEventFile{nullptr};
    TTree *fPromptTree{nullptr};
    TTree *fBgTree{nullptr};

    DetectorEvent fEvent;
    DetectorEvent *fEventBranch{&fEvent};
};

#endif
