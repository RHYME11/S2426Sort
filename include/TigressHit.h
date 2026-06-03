#ifndef __TIGRESSHIT_H__
#define __TIGRESSHIT_H__

#include <vector>

#include <Fragment.h>
#include <Rtypes.h>

class TigressHit {
  public:
    TigressHit();
    virtual ~TigressHit();

    // ============== Clear ==============
    // purpose: Clear stored TIGRESS fragments and array number.
    // inputs: none
    // outputs: none
    void Clear();

    // ============== Print ==============
    // purpose: Print array number and stored fragment name, energy, charge, and time.
    // inputs: none
    // outputs: none
    void Print() const;

    int ArryNumber() const { return fArryNumber; }
    void SetArryNumber(int arryNumber) { fArryNumber = arryNumber; }

    void AddCore(const Fragment& frag) { fCores.push_back(frag); }
    void AddSegment(const Fragment& frag) { fSegments.push_back(frag); }
    void AddBgo(const Fragment& frag) { fBgos.push_back(frag); }

    const std::vector<Fragment>& Cores() const { return fCores; }
    const std::vector<Fragment>& Segments() const { return fSegments; }
    const std::vector<Fragment>& Bgos() const { return fBgos; }
  
    void SetBGOFired();
    bool BGOFired() {return fBGOFired;}

  private:
    int fArryNumber{-1};
    bool fBGOFired{false};
    std::vector<Fragment> fCores;
    std::vector<Fragment> fSegments;
    std::vector<Fragment> fBgos;

  ClassDef(TigressHit, 1);
};

#endif
