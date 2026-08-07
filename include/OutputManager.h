#ifndef __OUTPUTMANAGER_H__
#define __OUTPUTMANAGER_H__

#include <string>

#include <Emma.h>
#include <Tigress.h>

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
    // purpose: Fill one EMMA/TIGRESS event into the selected event tree.
    // inputs: prompt flag, EMMA object, and TIGRESS object
    // outputs: none
    void FillEvent(bool prompt, const Emma& emma, const Tigress& tigress);

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

    Emma fEmma;
    Tigress fTigress;

    Emma *fEmmaBranch{&fEmma};
    Tigress *fTigressBranch{&fTigress};
};

#endif
