
#include<cstdio>
#include<chrono>

#include <TMidasBanks.h>
#include <TMidasFile.h>
#include <TMidasEvent.h>

#include <Fragment.h>
#include <EventBuilder.h>
#include <EventProcess.h>

#include <Histogramer.h>
#include <Channel.h>

#include <utils.h>

#include <coreMap.h>

void MakeTigressFragments(uint32_t*,int);
void MakeEmmaADC(uint32_t*,int);
void MakeEmmaTDC(uint32_t*,int);


//void doStatus(TMidasFile&,bool forcePrint=false);
void doStatus(TMidasFile&,bool forcePrint=false,bool drainingQueue=false);
auto start     = std::chrono::steady_clock::now();
auto lastPrint = std::chrono::steady_clock::now();
auto timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);
const std::chrono::seconds interval(1); // 1 second interval

static Long64_t xferhfts;     // Time stamp to be transferred from ADC to TDC; 10 ns ticks
int gl_counter1, gl_counter2, gl_counter3, gl_counter4;

int main(int argc, char **argv) {
  

  TMidasFile infile(argv[1]);
  TMidasEvent event;

  Histogramer *gHist = Histogramer::Get();

  int run,subrun;
  getRunNumber(argv[1],run,subrun);
  gHist->SetRun(run,subrun);

  ////printf(" sorting \t %s\n",argv[1]);
  ////printf(" \trun:    %i\n",run);
  ////printf(" \tsubrun: %i\n",subrun);

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal"); 

  //start event builder;
  EventBuilder::Get();
  //start event process;
  EventProcess::Get();

  std::map<std::string,int> banksFound;
  std::map<std::string,int> banksFound2;
  std::map<int,int>         typeFound;

  int counter = 0;
  while(infile.Read(event)) {
    //if(!event.GetEventId()&0xf000)
    //event.Print();
    switch(event.GetEventId()) {
      case 1:  //trigger
        xferhfts = 0;
        //printf("\n\n--------------------%i--------------------- \n",counter);
        Histogramer::Fill("EventTrigger",10,0,10,event.GetTriggerMask());
        event.SetBankList();
        //if(event.GetTriggerMask()==0) event.Print("all");
        void *ptr;
        int banksize;
        if((banksize = event.LocateBank(nullptr, "GRF4", &ptr)) > 0) {
          banksFound["GRIF4"]++;
          MakeTigressFragments((uint32_t*)ptr,banksize); 
        } else if((banksize = event.LocateBank(nullptr, "MADC", &ptr)) > 0) {
          //banksFound["MADC"]++;     // adc
          //printf(" ------------------------------------------ \n");
          MakeEmmaADC((uint32_t*)ptr,banksize); 
          if((banksize = event.LocateBank(nullptr, "EMMT", &ptr)) > 0) {   // MADC and EMMT are nested.
            MakeEmmaTDC((uint32_t*)ptr,banksize); 
            //banksFound["EMMT"]++;   // tdc
          }
          //event.Print("all");
          //printf("\n------------------------------------------ \n\n");
        } else if((banksize = event.LocateBank(nullptr, "EMMT", &ptr)) > 0) {
            MakeEmmaTDC((uint32_t*)ptr,banksize); 
            printf("EMMA TDC found out of MADC\n");          
        }
      case 2:  //scalar
      case 5:  //epics
        break;
      case 0x8000: //BOR (odb)
      case 0x8001: //This one should be ODB (EOR)
        event.Print();
        break;
      case 0x8002: //message Event;
        break;
    };
    typeFound[event.GetEventId()]++;
    counter++;
    doStatus(infile);
  } 
  EventBuilder::Get()->Stop();
  while(EventBuilder::Get()->Size() > 0) {
    doStatus(infile,true,true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  
  doStatus(infile,true,false);
  gHist->Close();
  EventProcess::Get()->PrintCounter();
  return 0;  
}


// ======================================================================================= // 
/*void doStatus(TMidasFile &infile,bool forcePrint) {
  
  if((std::chrono::steady_clock::now()-lastPrint) > interval) forcePrint=true;
  if(!forcePrint) return; 
  lastPrint = std::chrono::steady_clock::now();
  timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);
  printf(CLEAR_LINE);
  printf(" %lld \t read %.02f / %.02f  MB\r",
          timeEllapsed.count(),infile.GetBytesRead()/1024./1024.,
          infile.GetFileSize()/1024./1024.);
  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventBuilder[%s] Q[%i] pushed[%i]  pop[%i]\r",
           EventBuilder::Get()->Running() ?  "on" : "off",
           EventBuilder::Get()->Size(),EventBuilder::Get()->Pushed(),EventBuilder::Get()->Popped());
  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventProcess[%s] pushed[%i]\r",
           EventBuilder::Get()->Running() ?  "on" : "off",
           EventProcess::Get()->Pushed());

  if(infile.GetBytesRead()>=infile.GetFileSize()) {
    printf("\ndone!\n");
  } else {
    printf(CURSOR_UP);
    printf(CURSOR_UP);
  }
  fflush(stdout);
  //if(timeEllapsed.count()>20) break;
}*/

void doStatus(TMidasFile &infile, bool forcePrint, bool drainingQueue) {

  if((std::chrono::steady_clock::now()-lastPrint) > interval) forcePrint=true;
  if(!forcePrint) return;

  lastPrint = std::chrono::steady_clock::now();
  timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);

  printf(CLEAR_LINE);

  if(drainingQueue) {
    printf(" %lld \t finished reading %.02f / %.02f MB, waiting for EventBuilder queue to drain...\r",
            timeEllapsed.count(),
            infile.GetBytesRead()/1024./1024.,
            infile.GetFileSize()/1024./1024.);
  } else {
    printf(" %lld \t read %.02f / %.02f MB\r",
            timeEllapsed.count(),
            infile.GetBytesRead()/1024./1024.,
            infile.GetFileSize()/1024./1024.);
  }

  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventBuilder[%s] Q[%i] pushed[%i] pop[%i]\r",
          EventBuilder::Get()->Running() ? "on" : "off",
          EventBuilder::Get()->Size(),
          EventBuilder::Get()->Pushed(),
          EventBuilder::Get()->Popped());

  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventProcess[%s] pushed[%i]\r",
          EventProcess::Get()->Running() ? "on" : "off",
          EventProcess::Get()->Pushed());

  if(drainingQueue) {
    printf(CURSOR_DOWN);
    printf(CLEAR_LINE);
    printf("\t draining EventBuilder queue... remaining Q[%i]\r",
            EventBuilder::Get()->Size());
  }

  if(infile.GetBytesRead() >= infile.GetFileSize() && !drainingQueue) {
    printf("\ndone reading file!\n");
  } else if(drainingQueue && EventBuilder::Get()->Size() == 0) {
    printf("\nEventBuilder queue drained!\n");
  } else {
    printf(CURSOR_UP);
    printf(CURSOR_UP);
    if(drainingQueue) printf(CURSOR_UP);
  }

  fflush(stdout);
}

// ======================================================================================= // 
void MakeEmmaADC(uint32_t* pdata,int size) {
  long timestamp=0;
  int  channel=0;
  int  charge;
  while(size>0) { 
    size--;
    uint32_t datum = *(pdata+size);
    switch(datum&0xf0000000) { 
      case 0xc0000000:
      case 0xd0000000:
      case 0xe0000000:
      case 0xf0000000:
        timestamp=datum&0x3fffffff;
        break;   
      case 0x40000000:
        break;  // header event 0x4~0x7 
      case 0x00000000:
        if(datum & 0x00800000) {//
          timestamp += ((long(datum&0x0000ffff))<<30);
        } else if(datum & 0x04000000) {//
          channel = (datum>>16)&0x1F; // ADC Channel Number
          charge  = (datum & 0xfff); // ADC Charge

          std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
          frag.get()->AddCharge(charge);
          frag.get()->AddInt(5);
          frag.get()->SetAddress(0x800000 + channel);
          frag.get()->SetTimestamp(timestamp);
          frag.get()->SetCfd(0);             
          frag.get()->SetFilterPattern(0);   
          frag.get()->SetPileup(0);          
          frag.get()->SetDetType(13);
          frag.get()->SetTimestampUnit(50);
          xferhfts = frag.get()->Timestamp();
          int c     = frag.get()->Address()&0xff;
          float chg = frag.get()->Charge(); 
          //Histogramer::Get()->Fill("eADC",4000,0,64000,chg,
          //                                 1000,0,1000,c);
          //printf("%s(%i): %lu\n",frag.get()->Name().c_str(),frag.get()->DetType(),frag.get()->TimestampNs());
          EventBuilder::Get()->push(std::move(frag));
        }
        break;   
      default:  
        break;
    }
    //printf("0x%08x ",*(pdata+i));
    //if(i && (i%8)==0) printf("\n");
  }
}

// kludge stolen form  GH                0xffffffff 
static uint32_t wraparoundcounter = 0; //0xffffffff; // Needed for bad data at start of run before GRIFFIN starts
static uint32_t lasttimestamp = 0;     // "last" time stamp for simple wraparound algorightm 
static uint32_t countsbetweenwraps; // number of counts between wraparounds

// ======================================================================================= // 
void MakeEmmaTDC(uint32_t* pdata ,int size) {

  uint32_t tmpTimestamp = 0;
  uint32_t tmpAddress   = 0;
  Long64_t ts           = 0;

  std::vector<uint32_t> addresses;
  std::vector<uint32_t> charges;

  //for(int i=0;i<size;i++) {
  //  printf("0x%08x ",*(pdata+i));
  //  if(i && (i%8)==0) printf("\n");
  //}
  //printf("\n\n");


  for(int x=0;x<size;x++) {  
    uint32_t datum = *(pdata+x);  
    int type = (datum>>27) & 0x1f; //&0xf8000000) >>27;
                                   //printf("datum: %p\n",datum);
    switch(type) {
      case 0x8:  //event header
        break;
      case 0x1:  //tdc header
        tmpAddress = (datum>>12)&0xfff; // event ID 
        break;
      case 0x0:
        addresses.push_back((datum >> 21) & 0x1f); 
        charges.push_back(datum & 0x1fffff); // TDC MEASUREMENT
        break;
      case 0x3:  //tdc trailer
        break;
      case 0x4:  //tdc error
        break;
      case 0x11: //extended trigger time
        tmpTimestamp = (datum & 0x7FFFFFFU) << 5;
        break;
      case 0x10: //trigger time trailer
        tmpTimestamp |= ((datum) & 0x1fU);
        if(tmpTimestamp < lasttimestamp) {
          wraparoundcounter++;
          countsbetweenwraps=0;
        }
        lasttimestamp = tmpTimestamp;
        ts = static_cast<Long64_t>(lasttimestamp) +
          static_cast<Long64_t>(0x100000000ULL) * static_cast<Long64_t>(wraparoundcounter);
        ts = (ts * 5) >> 1; // before ts in 25ns, after ts in 10 ns;

        for (size_t i = 0; i < addresses.size(); ++i) {
          // optional duplicate check, mirroring GH:
          bool duped = false;
          //for (size_t j = 0; j < i; ++j) {
          //  if (addresses[i] == addresses[j]) {
          //    duped = true;
          //    break;
          //  }
          //}
          //if (duped) continue;
          std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
          frag.get()->AddCharge(charges.at(i));
          frag.get()->AddInt(5);
          frag.get()->SetAddress(0x900000 + addresses.at(i));
          if(xferhfts>0){
            frag.get()->SetTimestamp(xferhfts);
          }else {
            frag.get()->SetTimestamp(ts/5); // convert it to unit = 50ns 
            printf("no EMMA ADC in this event\n");
          }
          frag.get()->SetCfd(0);             
          frag.get()->SetFilterPattern(0);
          frag.get()->SetPileup(0);
          frag.get()->SetDetType(14);
          frag.get()->SetTimestampUnit(50);


           int c     = frag.get()->Address()&0xff;
           float chg = frag.get()->Charge(); 
           //Histogramer::Get()->Fill("eTDC",4000,0,64000,chg,
           //                                1000,0,1000,c);

          //printf("%s(%i): %lu\n",frag.get()->Name().c_str(),frag.get()->DetType(),frag.get()->TimestampNs());
          //printf("EMMA TDC\n");
          EventBuilder::Get()->push(std::move(frag));
        }
        addresses.clear();
        charges.clear();
        tmpTimestamp = 0;
        tmpAddress   = 0;
        break;
      default:
        break;
    }
  }
}

// ======================================================================================= // 
void MakeTigressFragments(uint32_t *pdata,int size) { 
  int words=0;
  int counter=0;
  int good=0;
  int bad=0;
  uint32_t *pStart = 0; 
  uint32_t *pEnd   = 0;
  while(words<size) {
    //printf("%p \n",*(pdata+words));
    if((*(pdata+words)&0xf0000000)==0x80000000)
      pStart = pdata+words;
    if((*(pdata+words)&0xf0000000)==0xe0000000)
      pEnd   = pdata+words;
    if(pStart!=0 && pEnd!=0) {
      counter++;
      int nwords = int(pEnd-pStart);
      std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
      int i=0;
      if(frag.get()->Unpack(pStart,nwords)) {
        //printf("CH%i: %i\n",frag.get()->Number(),frag.get()->DetType());
        good++;
        EventBuilder::Get()->push(std::move(frag));
//=========================================================================================================//
      }else{
        bad++;
      }
      pStart = 0;
      pEnd   = 0;
    }
    words+=1;
  }
  //printf("found %i frags\n",counter);
  //printf("\tgood: %i frags\n",good);
  //printf("\tbad: %i frags\n",bad);
}





































