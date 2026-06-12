#ifndef __EMMA_H__
#define __EMMA_H__


#include <limits>
#include <TObject.h>
#include <vector>

#include <Fragment.h>

class EmmaHit {
  public:
    EmmaHit() {  }
    EmmaHit(const Fragment&);
    ~EmmaHit() { } 

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    int Address() const { return fAddress; }
    int Number() const { return fNumber; }
    long Timestamp() const { return fTimestamp; }
    long TimestampNs() const { return fTimestampNs; }
    double Time() const { return fTime; }
    double Charge() const { return fCharge; }
    double Energy() const { return fEnergy; }

  private:
    int    fAddress{-1};
    int    fNumber{-1};
    long   fTimestamp{0};
    long   fTimestampNs{0};
    double fTime{0};
    double fCharge{0};
    double fEnergy{0};
    
  ClassDef(EmmaHit,2)
};

class Emma {
  public:
    Emma() { } 
    ~Emma() { } 

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    void AddADC(const Fragment& frag);
    void AddTDC(const Fragment& frag);
    void BuildHits();
    double CalculatePGACX() const;

  
    bool empty() const { return ((fADCTime==1) || (fTDCTime==-1)); }
    double ADCTime() const { return fADCTime; }
    double TDCTime() const { return fTDCTime; }
    double PGACX() const { return fPGACX; }

    const std::vector<EmmaHit>& ADC() const { return fADC; }
    const std::vector<EmmaHit>& TDC() const { return fTDC; }
    const std::vector<EmmaHit>& Si() const { return fSi; }
    const std::vector<EmmaHit>& IC1() const { return fIC1; }
    const std::vector<EmmaHit>& IC2() const { return fIC2; }
    const std::vector<EmmaHit>& IC3() const { return fIC3; }
    const std::vector<EmmaHit>& IC4() const { return fIC4; }
    const std::vector<EmmaHit>& Anodes() const { return fAnodes; }
    const std::vector<EmmaHit>& Left() const { return fLeft; }
    const std::vector<EmmaHit>& Right() const { return fRight; }
    const std::vector<EmmaHit>& Top() const { return fTop; }
    const std::vector<EmmaHit>& Bot() const { return fBot; }

    double fADCTime{1};
    double fTDCTime{-1};

  private:
    std::vector<EmmaHit> fADC;
    std::vector<EmmaHit> fTDC;

    std::vector<EmmaHit> fSi;
    std::vector<EmmaHit> fIC1;
    std::vector<EmmaHit> fIC2;
    std::vector<EmmaHit> fIC3;
    std::vector<EmmaHit> fIC4;

    std::vector<EmmaHit> fAnodes;

    std::vector<EmmaHit> fLeft;
    std::vector<EmmaHit> fRight;
    std::vector<EmmaHit> fTop;
    std::vector<EmmaHit> fBot;

    double fPGACX{std::numeric_limits<double>::quiet_NaN()};

  ClassDef(Emma,2)
};

#endif
