
#include <Fragment.h>
#include <TRandom.h>
#include <TVector3.h>

#include <globals.h>
#include <TigressGeometry.h>
#include <cstdint>

ClassImp(Fragment)

Fragment::Fragment() { } 

Fragment::~Fragment() { } 


// ============== Unpack ==============
// Purpose: unpack one GRF4 TIGRESS fragment from header through trailer.
// Inputs: data points to Word I; nwords is the trailer relative index.
// Outputs: returns true for a good fragment and fills fragment fields.
bool Fragment::Unpack(uint32_t *data,int &nwords) {
  if(data == nullptr || nwords <= 0) {
    return false;
  }

  auto packet = [](uint32_t word) {
    return word & 0xf0000000;
  };
  auto isNonPacket = [](uint32_t word) {
    return (word & 0x80000000) == 0;
  };
  auto signed25 = [](uint32_t word) {
    return int((word & 0x01ffffff) |
               (((word & 0x01000000) == 0x01000000) ? 0xfe000000 : 0x0));
  };

  // Word I: event header.
  int cword = 0;
  uint32_t datum = data[cword];
  int wordCount = (datum & 0x01f00000) >> 20;
  SetAddress((datum&0x000ffff0) >> 4);
  SetDetType((datum&0x0000000f) >> 0);
  SetTimestampUnit(10);

  cword++;

  // Word II: optional network packet after the header.
  if(cword >= nwords) {
    return false;
  }
  if(packet(data[cword]) == 0xd0000000) {
    cword++;
  }

  // Word III: optional primary filter pattern and pileup/waveform flags.
  if(cword >= nwords) {
    return false;
  }
  datum = data[cword];
  if((datum & 0xc0000000) == 0x00000000) {
    if(datum & 0x00008000) {
      SetHasWave();
    }
    SetFilterPattern((datum & 0x3fff0000) >> 16);
    SetPileup(datum & 0x0000001f);
    cword++;
  }

  // Word IV*: optional repeated filter IDs before the channel trigger ID.
  while(cword < nwords && isNonPacket(data[cword])) {
    cword++;
  }

  // Word V: channel trigger ID.
  if(cword >= nwords || packet(data[cword]) != 0x90000000) {
    return false;
  }
  int channelId = data[cword] & 0x0fffffff;
  cword++;

  // Word Va: optional network packet before the timestamp, matching GRSISort tolerance.
  if(cword >= nwords) {
    return false;
  }
  if(packet(data[cword]) == 0xd0000000) {
    cword++;
  }

  // Word VI: timestamp low bits.
  if(cword >= nwords) {
    return false;
  }
  if(packet(data[cword]) != 0xa0000000) {
    return false;
  }
  long timestamp = data[cword] & 0x0fffffff;
  cword++;

  // Word VII: timestamp high bits.
  if(cword >= nwords) {
    return false;
  }
  if(packet(data[cword]) != 0xb0000000) {
    return false;
  }
  timestamp += (long(data[cword] & 0x00003fff) << 28);
  SetTimestamp(timestamp);
  cword++;

  // Word VIIa*: optional waveform packets. Word III already set HasWave().
  if(cword >= nwords) {
    return false;
  }
  while(cword < nwords && packet(data[cword]) == 0xc0000000) {
    cword++;
  }

  // Word VIII: integration length high bits and pulse height.
  if(cword >= nwords || !isNonPacket(data[cword])) {
    return false;
  }
  datum = data[cword];
  int tempChg = signed25(datum);
  int tempInt = ((datum & 0x7c000000) >> 17) |
                (((datum & 0x40000000) == 0x40000000) ? 0xc000 : 0x0);
  cword++;

  // Word IX: integration length low bits and CFD.
  if(cword >= nwords || !isNonPacket(data[cword])) {
    return false;
  }
  datum = data[cword];
  SetCfd(datum & 0x003fffff);
  tempInt |= ((datum & 0x7fc00000) >> 22);
  cword++;

  // Word X-XIV: optional pileup/multi-charge words. Skip extras and keep Word VIII/IX charge.
  while(cword < nwords && isNonPacket(data[cword])) {
    cword++;
  }

  // Word XV: event trailer.
  if(cword != nwords || packet(data[cword]) != 0xe0000000) {
    return false;
  }
  if((data[cword] & 0x00003fff) != (channelId & 0x00003fff)) {
    return false;
  }
  (void)wordCount;
  AddInt(tempInt);
  AddCharge(tempChg);

  SetTheta();
  return true;
}

// =========== Print() ================= //
void Fragment::Print(Option_t *opt) const {
  Channel *c = Channel::Get(fAddress);
  printf("fragment @ 0x%016lx\n",fTimestamp);
  if(c)
  printf("\tname:        %s\n",c->Name().c_str());
  printf("\taddress:     0x%08x\n",fAddress);
  printf("\tchannel:     %i(%i)\n",   c->Number(),int(fAddress&0xff));
  //printf("\tchannel:     %i(%i)\n",   c->Number(),int((fAddress>>2)&0x1f));
  printf("\tdetType:     %i\n",fDetType);
  printf("\ttimestamp:   %lu\n",fTimestamp);//0x%08x\n",fCfd);
  printf("\ttimestampNS: %lu\n",fTimestamp*fTimestampUnit);//0x%08x\n",fCfd);
  //printf("\tcfd:         0x%08x\n",fCfd);
  printf("\tcfd:         %d\n",fCfd);//0x%08x\n",fCfd);
  printf("\ttime:        %.01f\n",Time());//0x%08x\n",fCfd);
  for(size_t i=0;i<fInt.size();i++) {
    printf("\tcharge[%lu]:   0x%08x\n",i,int(fCharge.at(i)));
    printf("\tcharge[%lu]:   %.02f\n",i,fCharge.at(i));
    printf("\tint[%lu]:      0x%08x\n",i,fInt.at(i));
    printf("\tEnergy[%lu]:   %.01f\n",i,Energy());
  }  
}

// =========== SetTheta() ================= //
void Fragment::SetTheta(){
  Channel *c = Channel::Get(fAddress); 
  std::string name = c->Name();
  if(name.length()<9) return;
  if(name.substr(0,3)!="TIG") return;
  int  det   = atoi(name.substr(3,2).c_str());
  char color = name.at(5);
  int  xtal  = (color == 'B') ? 0 :
               (color == 'G') ? 1 :
               (color == 'R') ? 2 :
               (color == 'W') ? 3 : -1;
  int  seg   = atoi(name.substr(7,2).c_str());
  double   xx = 0;
  double   yy = 0;
  double   zz = 0;
  switch(xtal){
    case 0:
      xx = TigressGeometry::GeBluePosition[det][seg][0];
      yy = TigressGeometry::GeBluePosition[det][seg][1];
      zz = TigressGeometry::GeBluePosition[det][seg][2]; 
      break;
    case 1:
      xx = TigressGeometry::GeGreenPosition[det][seg][0];
      yy = TigressGeometry::GeGreenPosition[det][seg][1];
      zz = TigressGeometry::GeGreenPosition[det][seg][2];
      break;
    case 2:
      xx = TigressGeometry::GeRedPosition[det][seg][0];
      yy = TigressGeometry::GeRedPosition[det][seg][1];
      zz = TigressGeometry::GeRedPosition[det][seg][2];
      break;
    case 3:
      xx = TigressGeometry::GeWhitePosition[det][seg][0];
      yy = TigressGeometry::GeWhitePosition[det][seg][1];
      zz = TigressGeometry::GeWhitePosition[det][seg][2];
      break;
    default:
      return;
  }
  TVector3 det_pos;
  det_pos.SetXYZ(xx, yy, zz);
  fTheta = det_pos.Theta();
}

// =========== Doppler() ================= //
double Fragment::Doppler(double beta) const{
  if(fTheta<0) return Energy();
  double gamma = 1 / (sqrt(1 - pow(beta, 2)));
  double tmp   = Energy() * gamma * (1 - beta * TMath::Cos(fTheta));
  return tmp;
}

// =========== Charge() ================= //
float Fragment::Charge()   const { // { return float(fCharge.at(0))/float(fInt.at(0)); }
  if(!fCharge.empty() && !fInt.empty())
    return float(fCharge.at(0))/(float(fInt.at(0)/5.)); 
  return -1;

}

// =========== Energy() ================= //
float Fragment::Energy() const {
  if(!fEnergy.empty()) 
    return fEnergy.at(0); 
  return -1;
}

// =========== AddCharge() ================= //
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

// ========== DetNumber() ============= //
int Fragment::DetNumber() const {
  std::string name = Channel::Get(fAddress)->Name();
  if(name.length()>5) return atoi(name.substr(3,2).c_str()); 
  return 99;
}

// ========== XtalNumber() ============= //
int Fragment::XtalNumber() const {
  std::string name = Channel::Get(fAddress)->Name();
  if (name.length() <= 5) return -1;                
  switch (name[5]) {                                
      case 'B': return 0;                           
      case 'G': return 1;                           
      case 'R': return 2;                           
      case 'W': return 3;                           
      default:  return -1;                          
  }
}

// ========== ArryNumber() ============= //
int Fragment::ArryNumber() const {
  std::string name = Channel::Get(fAddress)->Name();
  int det = 99;
  int xtal = -1;
  if (name.length() > 5) {
    det = std::stoi(name.substr(3,2))-1;
    switch (name[5]) {
    case 'B': xtal = 0; break;
    case 'G': xtal = 1; break;
    case 'R': xtal = 2; break;
    case 'W': xtal = 3; break;
    }
    return det * 4 + xtal+1;
  }
  return 99;
}
