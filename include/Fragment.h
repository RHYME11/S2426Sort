#ifndef __FRAGMENT_H__
#define __FRAGMENT_H__

#include <string>
#include <vector>

//#include <TObject.h>

#include <Channel.h>
#include <Rtypes.h>
#include "TRandom.h"

class Fragment { 
  public:
    Fragment();
    
    virtual ~Fragment();

    bool Unpack(uint32_t *pdata,int &nwords); 
    void Print(Option_t *opt="") const; 

    bool HasWave() const { return fHasWave; }


    // type == in tigress everything is a grif16 - so i do not care. 
    void SetAddress(int address)      { fAddress = address; }  
    void SetDetType(int detType)      { fDetType = detType; }
    void SetTimestamp(long timestamp) { fTimestamp = timestamp; UpdateTime(); }
    void SetHasWave()                 { fHasWave = !fHasWave; }
    void SetWaveSamples(int samples)  { fWaveSamples = samples; }
    void SetCfd(int cfd)              { fCfd = cfd; UpdateTime(); }
    void SetFilterPattern(int fp)     { fFilterPattern = fp; }    
    void SetPileup(int pileup)        { fPileup = pileup; }  
    void SetTimestampUnit(int timestampunit) { fTimestampUnit = timestampunit; UpdateTime(); }
    void AddCharge(int chg); 
    void AddInt(int i)                { fInt.push_back(i); }
    void SetTheta(); 

    int  Address()   const { return fAddress; }
    int  DetType()   const { return fDetType; }

    long Timestamp() const { return fTimestamp; }
    double Time()    const { return fTime; } 
    int  Cfd()       const { return fCfd;       }
    int  Filter()    const { return fFilterPattern; }
    int  Pileup()    const { return fPileup;        }

    float Charge()   const; 
    float Energy()   const; 

    int  Number()    const { return Channel::Get(fAddress)->Number(); } 
    int  DetNumber() const; // [5,16]
    int XtalNumber() const; // [0,3]
    int ArryNumber() const; // [21,64]
    std::string Name() const { return   Channel::Get(fAddress)->Name(); }  

    long TimestampNs()  const {return fTimestamp * fTimestampUnit;}
    double Theta() const {return fTheta;}
    double Doppler(double beta) const;    

    bool operator<(const Fragment& other) const { return TimestampNs()<other.TimestampNs();} // min_timestampNs frag at the top of priority_queue


  private:
    int fAddress{-1}; 
    int fDetType{-1}; 
  
    long fTimestamp{-1}; 
    int  fCfd{-1};
    int fFilterPattern{-1};
    int fPileup{-1};

    bool fHasWave{false};
    int  fWaveSamples{-1};

    int fTimestampUnit{-1};    
    double fTime{-1};

    std::vector<int> fInt;
    std::vector<float> fCharge;
    std::vector<float> fEnergy;
    double fTheta{-1};

    void UpdateTime() {
      if(fTimestamp < 0 || fCfd < 0 || fTimestampUnit < 0) return;
      fTime = double(fTimestamp & 0xfffffffffffc0000) * fTimestampUnit
            + double(fCfd + gRandom->Uniform()) / 1.6;
    }

  ClassDef(Fragment,1);
};


#endif
