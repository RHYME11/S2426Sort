#ifndef __TIGRESS_H__
#define __TIGRESS_H__

#include <array>
#include <string>
#include <vector>

#include <TObject.h>
#include <TVector3.h>

class DetectorEvent;
class Fragment;

class TigressChannelHit {
  public:
    TigressChannelHit() { }
    explicit TigressChannelHit(const Fragment& frag);
    virtual ~TigressChannelHit() { }

    virtual void Clear(Option_t *opt="");
    virtual void Print(Option_t *opt="") const;

    double Energy() const { return fEnergy; }
    double Charge() const { return fCharge; }
    int Address() const { return fAddress; }
    int Number() const { return fNumber; }
    const std::string& Name() const { return fName; }
    long Timestamp() const { return fTimestamp; }
    long TimestampNs() const { return fTimestampNs; }
    double Time() const { return fTime; }
    int CFD() const { return fCFD; }
    int KValue() const { return fInt; }

  private:
    double fEnergy{-1};
    double fCharge{-1};
    int fAddress{-1};
    int fNumber{-1};
    std::string fName;
    long fTimestamp{-1};
    long fTimestampNs{-1};
    double fTime{-1};
    int fCFD{-1};
    int fInt{-1};

  ClassDef(TigressChannelHit,2)
};

class TigressHit : public TigressChannelHit {
  public:
    TigressHit() { }
    explicit TigressHit(const Fragment& frag);
    ~TigressHit() override { }

    void Clear(Option_t *opt="") override;
    void Print(Option_t *opt="") const override;

    int DetectorNumber() const { return fDetectorNumber; }
    int ArrayNumber() const { return fArrayNumber; }
    bool BGOFire() const { return fBGOFire; }
    const TVector3& Position() const { return fPosition; }
    const std::vector<TigressChannelHit>& Segments() const { return fSegments; }

    double Doppler(double beta) const;

  private:
    friend class Tigress;

    void UpdatePosition();

    int fDetectorNumber{-1};
    int fArrayNumber{-1};
    bool fBGOFire{false};
    TVector3 fPosition;
    std::vector<TigressChannelHit> fSegments;

  ClassDefOverride(TigressHit,2)
};

class Tigress {
  public:
    static constexpr double SUPPRESSION_CHARGE = 20.0;
    static constexpr std::array<double, 2> SUPPRESSION_WINDOW_NS = {
      -300.0, 300.0
    };

    Tigress() { }
    virtual ~Tigress() { }

    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;

    void BuildHits(const DetectorEvent& event);
    void UpdateBGOFire(const std::array<double, 2>& timeWindow);

    const std::vector<TigressHit>& Hits() const { return fHits; }
    const std::vector<TigressChannelHit>& BGOHits() const { return fBGOHits; }

  private:
    std::vector<TigressHit> fHits;
    std::vector<TigressChannelHit> fBGOHits;

  ClassDef(Tigress,3)
};

#endif
