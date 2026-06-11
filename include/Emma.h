#ifndef __EMMA_H__
#define __EMMA_H__


#include <TObject.h>

//class Fragment;
#include <Fragment.h>

class EmmaHit {
  public:
    EmmaHit() {  }
    EmmaHit(Fragment&);
    ~EmmaHit() { } 

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

  //private:
    int    fAddress{-1};
    int    fId{-1};
    long   fTimestamp{0};
    double fTime{0};
    double fCharge{0};
    double fEcal{0};
    
  ClassDef(EmmaHit,1)
};

class Emma {
  public:
    Emma() { } 
    ~Emma() { } 

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    void BuildHits();

  
    bool empty() const { return ((fADCTime==1) || (fTDCTime==-1)); }

  //private:
    double fADCTime{1};
    double fTDCTime{-1};

    std::vector<Fragment> fADC;    //!
    std::vector<Fragment> fTDC;   //!

    EmmaHit fATop;
    EmmaHit fAMiddle;
    EmmaHit fABottom; 
    EmmaHit fSi;

    EmmaHit fIon1;
    EmmaHit fIon2;
    EmmaHit fIon3;
    EmmaHit fIon4;

    double fX;
    double fY;

    double fTrigger;
    double fRf;

  ClassDef(Emma,1)
};

#endif
