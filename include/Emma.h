#ifndef __EMMA_H__
#define __EMMA_H__

#include <limits>
#include <string>
#include <vector>

#include <DetectorEvent.h>
#include <Fragment.h>
#include <TObject.h>

class EmmaHit {
  public:
    EmmaHit() { }
    EmmaHit(const Fragment& frag);
    virtual ~EmmaHit() { }

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    int Address() const { return fAddress; }
    long Timestamp() const { return fTimestamp; }
    long TimestampNs() const { return fTimestampNs; }
    double Time() const { return fTime; }
    double Charge() const { return fCharge; }
    double Energy() const { return fEnergy; }
    const std::string& Name() const { return fName; }
    int Number() const { return fNumber; }

  private:
    int fAddress{-1};
    long fTimestamp{-1};
    long fTimestampNs{-1};
    double fTime{-1};
    double fCharge{-1};
    double fEnergy{-1};
    std::string fName;
    int fNumber{-1};

  ClassDef(EmmaHit,3)
};

class Emma {
  public:
    Emma() { }
    virtual ~Emma() { }

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;
    void BuildHits(const DetectorEvent& event);

    double PGACX() const { return fPGACX; }

    const std::vector<EmmaHit>& Si() const { return fSi; }
    const std::vector<EmmaHit>& IC0() const { return fIC0; }
    const std::vector<EmmaHit>& IC1() const { return fIC1; }
    const std::vector<EmmaHit>& IC2() const { return fIC2; }
    const std::vector<EmmaHit>& IC3() const { return fIC3; }
    const std::vector<EmmaHit>& Anode() const { return fAnode; }
    const std::vector<EmmaHit>& Left() const { return fLeft; }
    const std::vector<EmmaHit>& Right() const { return fRight; }
    const std::vector<EmmaHit>& Top() const { return fTop; }
    const std::vector<EmmaHit>& Bot() const { return fBot; }

  private:
    double CalculatePGACX() const;

    std::vector<EmmaHit> fSi;
    std::vector<EmmaHit> fIC0;
    std::vector<EmmaHit> fIC1;
    std::vector<EmmaHit> fIC2;
    std::vector<EmmaHit> fIC3;

    std::vector<EmmaHit> fAnode;
    std::vector<EmmaHit> fLeft;
    std::vector<EmmaHit> fRight;
    std::vector<EmmaHit> fTop;
    std::vector<EmmaHit> fBot;

    double fPGACX{std::numeric_limits<double>::quiet_NaN()};

  ClassDef(Emma,3)
};

#endif
