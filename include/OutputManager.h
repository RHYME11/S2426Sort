#ifndef __OUTPUTMANAGER_H__
#define __OUTPUTMANAGER_H__

#include <mutex>
#include <string>

#include <DetectorEvent.h>
#include <Emma.h>
#include <Fragment.h>
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
    // Purpose: Open requested TTree ROOT files and create their trees.
    // Inputs: Run, subrun, and fragment-only selection.
    // Outputs: None.
    void Open(int run, int subrun, bool fragmentOnly = false);

    // ============== FillEvent ==============
    // Purpose: Fill one complete detector event into EventTree.
    // Inputs: Detector event.
    // Outputs: None.
    void FillEvent(const DetectorEvent& event);

    // ============== FillPhysics ==============
    // Purpose: Fill one Emma/Tigress pair into exactly one Physics tree.
    // Inputs: Built Emma and Tigress objects.
    // Outputs: None.
    void FillPhysics(const Emma& emma, const Tigress& tigress);

    // ============== FillFragment ==============
    // Purpose: Fill one time-ordered fragment before it leaves EventBuilder.
    // Inputs: Fragment still owned by the EventBuilder queue.
    // Outputs: None.
    void FillFragment(const Fragment& fragment);

    // ============== Close ==============
    // Purpose: Write all trees and close the three TTree ROOT files.
    // Inputs: None.
    // Outputs: None.
    static void Close();

  private:
    std::string fFragmentFilename;
    std::string fEventFilename;
    std::string fPhysicsFilename;

    TFile *fFragmentFile{nullptr};
    TFile *fEventFile{nullptr};
    TFile *fPhysicsFile{nullptr};

    TTree *fFragmentTree{nullptr};
    TTree *fEventTree{nullptr};
    TTree *fBgTree{nullptr};
    TTree *fPromptGoodTree{nullptr};
    TTree *fPromptBadTree{nullptr};

    Fragment fFragment;
    DetectorEvent fEvent;
    Emma fEmma;
    Tigress fTigress;

    Fragment *fFragmentBranch{&fFragment};
    DetectorEvent *fEventBranch{&fEvent};
    Emma *fEmmaBranch{&fEmma};
    Tigress *fTigressBranch{&fTigress};

    std::mutex fMutex;
};

#endif
