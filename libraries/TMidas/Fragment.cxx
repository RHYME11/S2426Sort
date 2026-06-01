
#include <Fragment.h>
#include <TRandom.h>
#include <TVector3.h>

#include <globals.h>
#include <TigressGeometry.h>
#include <cstdint>

ClassImp(Fragment)

Fragment::Fragment() { } 

Fragment::~Fragment() { } 


//int Fragment::Unpack(char *data) {
//unpacks assuming grf4 data format
bool Fragment::Unpack(uint32_t *data,int &nwords) {
  
  //for(int i=0;i<nwords;i++) {
  //  printf("%p ",(data+nwords));
  //}
  //printf("\n");

  int cword =0; // points to header;
  //printf("%p \t %i\n",data,nwords);  
  //return;
  uint32_t datum = *(data+cword);
  SetAddress((datum&0x000ffff0) >> 4);
  SetDetType((datum&0x0000000f) >> 0);

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

  //for(int i=0;i<nwords;i++) {
  //  printf("%p ",*(data+i));
  //}
  //printf("\n");

  datum = *(data+cword);
  if((datum&0xf0000000)!=0xa0000000)  { return false;
    //printf("bad first timestamp word! %p \n",datum);
    //for(int i=0;i<nwords;i++) {
    //  printf("%p ",*(data+i));
    //}
    //printf("\n");
  }
  long timestamp = (datum & 0x0fffffff);
  cword+=1; //advance to vi

  // --- Read Chunk VII (Timestamp High) ---
  datum = *(data+cword);
  if((datum&0xf0000000)!=0xb0000000)  { return false; } //printf("bad second timestamp word! %p \n",datum); } 
  timestamp += (long(datum & 0x00003fff) << 28);
  SetTimestamp(timestamp);
  cword+=1;

  // --- Read Chunk VIII (Charge/Waveform Check) ---
  if ((datum & 0xf0000000) == 0xc0000000) {
    // Process waveform data words until the signature changes
    while ((datum & 0xf0000000) == 0xc0000000) {
      // (Waveform data processing would go here)
      datum = *(data+cword);
      cword+=1;
    }
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
  SetTheta();
  //Print();
  return true;
  //return bytes_processed;
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
  if(seg==0) seg = 8; // sge=0 ==> "00A or 00B", then set seg = 8, got core from TigressGeometry
  double   xx = 0;
  double   yy = 0;
  double   zz = 0;
  switch(xtal){
    case 0:
      xx = TigressGeometry::GeBluePositionBack[det][seg][0];
      yy = TigressGeometry::GeBluePositionBack[det][seg][1];
      zz = TigressGeometry::GeBluePositionBack[det][seg][2]; 
      break;
    case 1:
      xx = TigressGeometry::GeGreenPositionBack[det][seg][0];
      yy = TigressGeometry::GeGreenPositionBack[det][seg][1];
      zz = TigressGeometry::GeGreenPositionBack[det][seg][2];
      break;
    case 2:
      xx = TigressGeometry::GeRedPositionBack[det][seg][0];
      yy = TigressGeometry::GeRedPositionBack[det][seg][1];
      zz = TigressGeometry::GeRedPositionBack[det][seg][2];
      break;
    case 3:
      xx = TigressGeometry::GeWhitePositionBack[det][seg][0];
      yy = TigressGeometry::GeWhitePositionBack[det][seg][1];
      zz = TigressGeometry::GeWhitePositionBack[det][seg][2];
      break;
    default:
      return;
  }
  TVector3 det_pos;
  det_pos.SetXYZ(xx, yy, zz);
  fTheta = det_pos.Theta();
}

// =========== SetTheta() ================= //
double Fragment::Doppler(double beta){
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
    det = std::stoi(name.substr(3,2));
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



