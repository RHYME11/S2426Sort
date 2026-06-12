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

    double fADCTime{1};
    double fTDCTime{-1};

  //private:
    std::vector<Fragment> fADC;    //!
    std::vector<Fragment> fTDC;   //!

    std::vector<EmmaHit> fAnode;
    EmmaHit fSi;

    std::vector<EmmaHit> fIon1;
    std::vector<EmmaHit> fIon2;
    std::vector<EmmaHit> fIon3;
    std::vector<EmmaHit> fIon4;

    std::vector<EmmaHit> fLeft;
    std::vector<EmmaHit> fRight;
    std::vector<EmmaHit> fTop;
    std::vector<EmmaHit> fBot;

    double fPGACX{std::numeric_limits<double>::quiet_NaN()};

  ClassDef(Emma,1)
};

#endif
