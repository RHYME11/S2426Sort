
#include<DetectorProcess.h>

#include<EventBuilder.h>
#include<Histogramer.h>

#include<globals.h>


#include <map>
#include <algorithm>
#include <utility>


DetectorProcess *DetectorProcess::fDetectorProcess = 0;

DetectorProcess::DetectorProcess() {
  fWorker = std::thread([this]{ this->loop(); });
  fWorker.detach();
}

DetectorProcess *DetectorProcess::Get() {
  if(!fDetectorProcess)
    fDetectorProcess = new DetectorProcess;
  return fDetectorProcess;
}

DetectorProcess::~DetectorProcess() { 
  printf("DetectorProcess destructor called; fStop[%i]\n",fStop.load());
  if(!fStop)
    fWorker.join();
}


void DetectorProcess::loop() {
  while(1) {
    if(fStop) break;

    DetectorEvent event;

    if(!EventProcess::Get()->pop(event)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    fPushed++;

    // -------------------------
    // TIGRESS singles
    // -------------------------
    if(event.tigress) {
      //for(auto& hit : event.tigress->Hits()) {
      // Histogramer::Fill("tigress_energy",
      //                    4000, 0, 4000,
      //                    hit.Energy());

      //  Histogramer::Fill("tigress_time",
      //                    4000, -2000, 2000,
      //                    hit.Time() - event.timestamp);
      //}
      for(auto& current: event.tigress->fCoreHits) {
        int  det   = std::stoi(current.Name().substr(3,2));
        char color = current.Name().at(5);

        int xtal = (color == 'R') ? 0 :
          (color == 'G') ? 1 :
          (color == 'B') ? 2 :
          (color == 'W') ? 3 : -1;

        Histogramer::Fill("summary",70,0,70,
            det*4 + xtal,
            8000,0,4000,
            current.Energy());
      }
    }

    // -------------------------
    // EMMA singles
    // -------------------------

    if(event.emma) {
      //for(auto& hit : event.emma->Hits()) {
      if(!event.emma->empty()) {
        Histogramer::Fill("emma_adc_tdc_time",
            1000, -1000, 1000,
            event.emma->fADCTime - event.emma->fTDCTime);
        //}
      }
    }

    // -------------------------
    // TIGRESS-EMMA coincidences
    // -------------------------
    
       if(event.tigress && event.emma) {
       
        for(auto& current: event.tigress->fCoreHits) {
          Histogramer::Fill("emma_tig_dt",2000,-1000,1000,event.emma->fADCTime - current.Timestamp());

        }

       }
  }
}






