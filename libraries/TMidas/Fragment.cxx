
#include <Fragment.h>
#include <TRandom.h>

#include <globals.h>
#include <cstdint>

ClassImp(Fragment)

Fragment::Fragment() { } 

Fragment::~Fragment() { } 


// ============== Unpack ==============
// Purpose: Decode GRF4 fields and save waveform samples for TIP addresses.
// Inputs: Raw fragment words and fragment length.
// Outputs: Decoded fields; false if timestamp words are invalid.
bool Fragment::Unpack(uint32_t *data,int &nwords) {

  int cword =0; // points to header;
  uint32_t datum = *(data+cword);
  SetAddress((datum&0x000ffff0) >> 4);
  SetDetType((datum&0x0000000f) >> 0);

  fWaveform.clear();
  const bool isTip =
    (fAddress & 0xf000) == 0x4000 || (fAddress & 0xf000) == 0x5000;

  //network packet
  cword+=1; // points to network packet? 
  datum = *(data+cword);
  if((datum&0xf0000000) == 0xd0000000) { //yes.
    cword+=1;
  }

  //filter pattern;
  datum = *(data+cword);
  if(datum&0x00008000) SetHasWave();
  SetFilterPattern((datum&0x3fff0000)>>16);
  SetPileup(datum&0x0000001f);

  cword+=1; //advance to iv
  cword+=1; //advance to v

  datum = *(data+cword);
  if((datum&0xf0000000)!=0xa0000000)  {
    cword+=1; //advance to v
    datum = *(data+cword);
    if((datum&0xf0000000)!=0xa0000000)  {
      cword+=1; //advance to v
      datum = *(data+cword);
    }
  }
  datum = *(data+cword);
  if((datum&0xf0000000)!=0xa0000000)  { return false;}
  long timestamp = (datum & 0x0fffffff);
  cword+=1; //advance to vi

  // --- Read Chunk VII (Timestamp High) ---
  datum = *(data+cword);
  if((datum&0xf0000000)!=0xb0000000)  { return false; } //printf("bad second timestamp word! %p \n",datum); } 
  timestamp += (long(datum & 0x00003fff) << 28);
  SetTimestamp(timestamp);
  cword+=1;

  // --- Read Chunk VIII (Charge/Waveform Check) ---
  datum = *(data+cword);
  while ((datum & 0xf0000000) == 0xc0000000) {
    if (isTip) {
      // Each word contains two signed 14-bit samples.
      int sample0 = datum & 0x3fff;
      int sample1 = (datum >> 14) & 0x3fff;
      if (sample0 & 0x2000) {
        sample0 -= 0x4000;
      }
      if (sample1 & 0x2000) {
        sample1 -= 0x4000;
      }
      fWaveform.push_back(static_cast<Short_t>(sample0));
      fWaveform.push_back(static_cast<Short_t>(sample1));
    }
    cword+=1;
    datum = *(data+cword);
  }

  // --- Parse Charge and Integration Values ---
  datum = *(data+cword);
  int tempChg = (datum & 0x3ffffff);
  // Bit manipulation adjusted for clearer casting/shifting:
  int tempInt = (datum & 0x7c000000) >> (26-9); 

  cword+=1;
  // --- Read Chunk IX (CFD/More Integration Values) ---
  datum = *(data+cword);

  SetCfd(datum & 0x003fffff);
  tempInt += ((datum & 0x7fc00000) >> 22);
  AddInt(tempInt);
  AddCharge(tempChg);
  SetTimestampUnit(10);
  return true;
  //return bytes_processed;
}

void Fragment::Print(Option_t *opt) const {
  Channel *c = Channel::Get(fAddress);
  printf("fragment @ 0x%016lx\n",fTimestamp);
  if(c)
  printf("\tname:        %s\n",c->Name().c_str());
  printf("\taddress:     0x%08x\n",fAddress);
  printf("\tdetType:     %i\n",fDetType);
  printf("\ttimestamp:   %lu\n",fTimestamp);//0x%08x\n",fCfd);
  printf("\tcfd:         0x%08x\n",fCfd);
  printf("\tcfd:         %d\n",fCfd);//0x%08x\n",fCfd);
  printf("\ttime:        %.01f\n",Time());//0x%08x\n",fCfd);
  for(size_t i=0;i<fInt.size();i++) {
    printf("\tcharge[%lu]:   0x%08x\n",i,int(fCharge.at(i)));
    printf("\tcharge[%lu]:   %.02f\n",i,fCharge.at(i));
    printf("\tint[%lu]:      0x%08x\n",i,fInt.at(i));
    printf("\tEnergy[%lu]:   %.01f\n",i,Energy());
  }  
}


float Fragment::Charge()   const { // { return float(fCharge.at(0))/float(fInt.at(0)); }
  if(!fCharge.empty() && !fInt.empty())
    return float(fCharge.at(0))/(float(fInt.at(0)/5.)); 
  return -1;

}

float Fragment::Energy() const {
  if(!fEnergy.empty()) 
    return fEnergy.at(0); 
  return -1;
}

void Fragment::AddCharge(int charge) {
  float chg = float(charge) +  gRandom->Uniform();
  fCharge.push_back(chg);
  int order = 0;
  float eng = 0;
  for(auto i : Channel::Get(fAddress)->CalPars()) {  
    eng += pow(Charge(),order++)*i;
    //if((fAddress&0x000f)==0x0009 && (Channel::Get(fAddress)->Number()<710))
    //  printf("%p  %i: eng: %.2f\t Charge() = %.2f \n",fAddress,order-1,eng,Charge());
  }
  //printf("eng = %.02f \n", eng); 
  fEnergy.push_back(eng);
}


