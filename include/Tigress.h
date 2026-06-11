#ifndef __TIGRESS_H__
#define __TIGRESS_H__


#include <TObject.h>

//class Fragment;
#include <Fragment.h>

class TigressHit {
  public:
    TigressHit() {  }
    TigressHit(Fragment&);
    ~TigressHit() { } 

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

  //private:
    int    fAddress{-1};
    int    fId{-1};
    long   fTimestamp{0};
    double fTime{0};
    double fCharge{0};
    double fEcal{0};
    
  ClassDef(TigressHit,1)
};

class Tigress {
  public:
    Tigress() { } 
    ~Tigress() { } 

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    void BuildHits();

  //private:
    std::vector<Fragment> fCoreHits;    //!
    std::vector<Fragment> fSegmentHits; //!
    std::vector<Fragment> fBGOHits;     //!

    std::vector<TigressHit> fHits;

  ClassDef(Tigress,1)
};

#endif
