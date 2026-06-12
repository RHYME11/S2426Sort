
#include<cstdio>
#include<chrono>

#include <TMidasBanks.h>
#include <TMidasFile.h>
#include <TMidasEvent.h>

#include <Fragment.h>
#include <EventBuilder.h>
#include <EventProcess.h>
#include <DetectorProcess.h>

#include <Histogramer.h>
#include <Channel.h>

#include <utils.h>

#include <coreMap.h>

void MakeTigressFragments(uint32_t*,int);
long MakeEmmaADC(uint32_t*,int);
void MakeEmmaTDC(uint32_t*,int,long);


void doStatus(TMidasFile&,bool forcePrint=false);
auto start     = std::chrono::steady_clock::now();
auto lastPrint = std::chrono::steady_clock::now();
auto timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);
const std::chrono::seconds interval(1); // 1 second interval

constexpr long EMMA_TO_TIGRESS_TS_SCALE = 5;

int main(int argc, char **argv) {
  TMidasFile infile(argv[1]);
  TMidasEvent event;

  Histogramer *gHist = Histogramer::Get();

  int run,subrun;
  getRunNumber(argv[1],run,subrun);
  gHist->SetRun(run,subrun);

  printf(" sorting \t %s\n",argv[1]);
  printf(" \trun:    %i\n",run);
  printf(" \tsubrun: %i\n",subrun);

  Channel::Read("cal/CalibrationFile_Nov182025.cal"); 

  //start event builder;
  EventBuilder::Get();
  //start event process;
  EventProcess::Get();
  //start detector process;
  DetectorProcess::Get();


  std::map<std::string,int> banksFound;
  std::map<std::string,int> banksFound2;
  std::map<int,int>         typeFound;

  int counter = 0;
  while(infile.Read(event)) {

    switch(event.GetEventId()) {
      case 1: { //trigger
                event.SetBankList();
                void *ptr;
                int banksize;

                long emmaAdcTimestamp = 0;
                bool haveEmmaAdcTimestamp = false; 

                if((banksize = event.LocateBank(nullptr, "GRF4", &ptr)) > 0) {
                  banksFound["GRIF4"]++;
                  MakeTigressFragments((uint32_t*)ptr,banksize); 
                } 
                if((banksize = event.LocateBank(nullptr, "MADC", &ptr)) > 0) {
                  banksFound["MADC"]++;     // adc
                  emmaAdcTimestamp = MakeEmmaADC((uint32_t*)ptr,banksize);
                  haveEmmaAdcTimestamp = (emmaAdcTimestamp > 0);
                }
                if((banksize = event.LocateBank(nullptr, "EMMT", &ptr)) > 0) {   // MADC and EMMT are nested.
                  banksFound["EMMT"]++;   // tdc
                  if(!haveEmmaAdcTimestamp) 
                    printf(RED "EMMA TDC without ADC" RESET_COLOR "\n");
                  MakeEmmaTDC((uint32_t*)ptr,banksize,emmaAdcTimestamp); 
                }
                break;
              }
      case 2:  //scalar
      case 5:  //epics
              break;
      case 0x8000: //BOR (odb)
      case 0x8001: //EOR
              event.Print();
              break;
      case 0x8002: //message Event;
              break;
    };
    typeFound[event.GetEventId()]++;
    counter++;
    doStatus(infile);

  }
  doStatus(infile,true);
  printf("\nDraining queues...\n");

  EventBuilder::Get()->Flush();

  while(EventBuilder::Get()->Size() > 0 || EventProcess::Get()->Size() > 0) {
    doStatus(infile, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  EventBuilder::Get()->Stop();
  EventProcess::Get()->Stop();
  DetectorProcess::Get()->Stop();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  printf("\nFinal status:\n");
  doStatus(infile, true);

  gHist->Close();
  return 0;
}

// =================================================================================== //
void doStatus(TMidasFile &infile, bool forcePrint) {

  if((std::chrono::steady_clock::now()-lastPrint) > interval) forcePrint=true;
  if(!forcePrint) return;

  lastPrint = std::chrono::steady_clock::now();
  timeEllapsed = std::chrono::duration_cast<std::chrono::seconds>(lastPrint-start);

  const double readMB  = infile.GetBytesRead()/1024./1024.;
  const double totalMB = infile.GetFileSize()/1024./1024.;
  const double frac    = infile.GetFileSize() > 0 ? readMB/totalMB : 0.0;
  const double rate    = timeEllapsed.count() > 0 ? readMB/timeEllapsed.count() : 0.0;

  const bool doneReading = infile.GetBytesRead() >= infile.GetFileSize();

  if(doneReading) {
    printf("\n");
    printf(" %lld s  read %.02f / %.02f MB  %.1f%%  %.2f MB/s\n",
        timeEllapsed.count(), readMB, totalMB, 100.0*frac, rate);

    printf("\t EventBuilder[%s] Q[%u] fragments_in[%u] built_events_out[%u]\n",
        EventBuilder::Get()->Running() ? "on" : "off",
        EventBuilder::Get()->Size(),
        EventBuilder::Get()->Pushed(),
        EventBuilder::Get()->Popped());

    printf("\t EventProcess [%s] Q[%u] detector_events_ready[%u] built_events_in[%u]\n",
        EventProcess::Get()->Running() ? "on" : "off",
        EventProcess::Get()->Size(),
        EventProcess::Get()->Size(),
        EventProcess::Get()->Pushed());

    printf("\t DetectorProcess[%s] detector_events_done[%u]\n",
        DetectorProcess::Get()->Running() ? "on" : "off",
        DetectorProcess::Get()->Pushed());

    fflush(stdout);
    return;
  }

  printf(CLEAR_LINE);
  printf(" %lld s  read %.02f / %.02f MB  %.1f%%  %.2f MB/s\r",
      timeEllapsed.count(), readMB, totalMB, 100.0*frac, rate);

  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventBuilder[%s] Q[%u] fragments_in[%u] built_events_out[%u]\r",
      EventBuilder::Get()->Running() ? "on" : "off",
      EventBuilder::Get()->Size(),
      EventBuilder::Get()->Pushed(),
      EventBuilder::Get()->Popped());

  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t EventProcess [%s] Q[%u] detector_events_ready[%u] built_events_in[%u]\r",
      EventProcess::Get()->Running() ? "on" : "off",
      EventProcess::Get()->Size(),
      EventProcess::Get()->Size(),
      EventProcess::Get()->Pushed());

  printf(CURSOR_DOWN);
  printf(CLEAR_LINE);
  printf("\t DetectorProcess[%s] detector_events_done[%u]\r",
      DetectorProcess::Get()->Running() ? "on" : "off",
      DetectorProcess::Get()->Pushed());

  printf(CURSOR_UP);
  printf(CURSOR_UP);
  printf(CURSOR_UP);

  fflush(stdout);
}


// =================================================================================== //
long MakeEmmaADC(uint32_t* pdata,int size) {
  //printf("MADC, size = %i:\n",size);
  long timestamp=0;
  long lastGoodTimestamp=0;
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
        break;   
      case 0x00000000:
        if(datum & 0x00800000) {//
          timestamp += ((long(datum&0x0000ffff))<<30);
          timestamp = timestamp * EMMA_TO_TIGRESS_TS_SCALE;
        } else if(datum & 0x04000000) {//
          channel = (datum>>16)&0x1F; // ADC Channel Number
          charge  = (datum & 0xfff); // ADC Charge

          std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
          frag.get()->AddCharge(charge);
          frag.get()->AddInt(5);
          frag.get()->SetAddress(0x800000 + channel);
          frag.get()->SetTimestamp(timestamp);
          lastGoodTimestamp = timestamp;
          frag.get()->SetCfd(0);             
          frag.get()->SetFilterPattern(0);   
          frag.get()->SetPileup(0);
          frag.get()->SetTimestampUnit(50);          
          frag.get()->SetDetType(13);
          EventBuilder::Get()->push(std::move(frag));
        }
        break;   
      default:  
        break;
    }
    //printf("0x%08x ",*(pdata+i));
    //if(i && (i%8)==0) printf("\n");
  }
  //printf("\n");
  return lastGoodTimestamp;
}

// kludge stolen form  GH                0xffffffff 
static uint32_t wraparoundcounter = 0; //0xffffffff; // Needed for bad data at start of run before GRIFFIN starts
static uint32_t lasttimestamp = 0;     // "last" time stamp for simple wraparound algorightm 
static uint32_t countsbetweenwraps; // number of counts between wraparounds
// ======================================================================== //
void MakeEmmaTDC(uint32_t* pdata ,int size, long adcTimestamp) {
  uint32_t tmpTimestamp = 0;
  uint32_t tmpAddress   = 0;
  Long64_t ts           = 0;

  std::vector<uint32_t> addresses;
  std::vector<uint32_t> charges;

  for(int x=0;x<size;x++) {  
    uint32_t datum = *(pdata+x);  
    int type = (datum>>27) & 0x1f; //&0xf8000000) >>27;
                                   //printf("datum: %p\n",datum);
    switch(type) {
      case 0x8:  //event header
        break;
      case 0x1:  //tdc header
        tmpAddress = (datum>>16)&0x300; 
        break;
      case 0x0:  //tdc measurement
        addresses.push_back((datum >> 21) & 0x1f);  
        charges.push_back(datum & 0x1fffff);
        break;
      case 0x3:  //tdc trailer
        break;
      case 0x4:  //tdc error
        break;
      case 0x11: //extended trigger time
        tmpTimestamp = (datum & 0x7FFFFFFU) << 5;
        break;
      case 0x10: //trailer
        tmpTimestamp |= ((datum) & 0x1fU);
        if(tmpTimestamp < lasttimestamp) {
          wraparoundcounter++;
          countsbetweenwraps=0;
        }
        lasttimestamp = tmpTimestamp;
        ts = static_cast<Long64_t>(lasttimestamp) +
          static_cast<Long64_t>(0x100000000ULL) * static_cast<Long64_t>(wraparoundcounter);
        ts = (ts * 5) >> 1;

        for (size_t i = 0; i < addresses.size(); ++i) {
          bool duped = false;
          std::unique_ptr<Fragment> frag = std::make_unique<Fragment>();
          frag.get()->AddCharge(charges.at(i));
          frag.get()->AddInt(5);
          frag.get()->SetAddress(addresses.at(i));
          frag.get()->SetTimestamp(adcTimestamp);
          frag.get()->SetCfd(0);             
          frag.get()->SetFilterPattern(0);
          frag.get()->SetPileup(0);
          frag.get()->SetDetType(14);
          frag.get()->SetTimestampUnit(50);
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

void MakeTigressFragments(uint32_t *pdata,int size) { 
  int words=0;
  int counter=0;
  int good=0;
  int bad=0;
  uint32_t *pStart = 0; 
  uint32_t *pEnd   = 0;
  while(words<size) {
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
        good++;
        Histogramer::Fill("GRF4","DetType",20,0,20,frag.get()->DetType());
        EventBuilder::Get()->push(std::move(frag));
      } else {
        bad++;
      }
      pStart = 0;
      pEnd   = 0;
    }
    words+=1;
  }
}





































