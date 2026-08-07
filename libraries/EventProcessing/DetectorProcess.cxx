
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
      for(auto& current: event.tigress->fCoreHits) {
        int  det   = std::stoi(current.Name().substr(3,2));
        char color = current.Name().at(5);

        int xtal = (color == 'R') ? 0 :
          (color == 'G') ? 1 :
          (color == 'B') ? 2 :
          (color == 'W') ? 3 : -1;

        Histogramer::Fill("summary",70,0,70,det*4 + xtal,8000,0,4000,current.Energy());
      }
    }

    // -------------------------
    // EMMA singles
    // -------------------------

    if(event.emma) {
      if(!event.emma->empty()) {
        Histogramer::Fill("emma_adc_tdc_time", 1000, -1000, 1000, event.emma->ADCTime() - event.emma->TDCTime());
      }
    }

    // -------------------------
    // TIGRESS-EMMA coincidences
    // -------------------------
    
    if(event.tigress && event.emma) {
      bool IsGood = false;
      if(event.emma->Si().size()>0 
      && event.emma->Anodes().size()>0 
      && (event.emma->Left().size()>0||event.emma->Right().size()>0)) {IsGood = true;}
      for(auto& current: event.tigress->fCoreHits) {
        int  det   = std::stoi(current.Name().substr(3,2));
        char color = current.Name().at(5);

        int xtal = (color == 'R') ? 0 :
          (color == 'G') ? 1 :
          (color == 'B') ? 2 :
          (color == 'W') ? 3 : -1;
        Histogramer::Fill("Emma_Tig","summary",70,0,70,det*4 + xtal,8000,0,4000,current.Energy());
        Histogramer::Fill("Emma_Tig","emma_tig_dt",2000,-10000,10000,event.emma->ADCTime() - current.TimestampNs());
        if(IsGood){
          Histogramer::Fill("Emma_Tig","summary_good",70,0,70,det*4 + xtal,8000,0,4000,current.Energy());
        }
      } // loop tig core over
      Histogramer::Fill("Emma_Tig","Si Size",100,0,100,event.emma->Si().size());
      Histogramer::Fill("Emma_Tig","Anode Size",100,0,100,event.emma->Anodes().size());
      Histogramer::Fill("Emma_Tig","IC1 Size"  ,100,0,100,event.emma->IC1().size());
      Histogramer::Fill("Emma_Tig","IC2 Size"  ,100,0,100,event.emma->IC2().size());
      Histogramer::Fill("Emma_Tig","IC3 Size"  ,100,0,100,event.emma->IC3().size());
      Histogramer::Fill("Emma_Tig","IC4 Size"  ,100,0,100,event.emma->IC4().size());
      if(event.emma->Si().size()>0){
        Histogramer::Fill("Emma_Tig/Si_triggered", "Anode Size", 10,0,10,event.emma->Anodes().size());
      }
    } // tig-emma coinc

    // -------------------------
    // Prompt
    // -------------------------
  
    if(event.prompt){
      Histogramer::Fill("EventProcessing","prompt: Core.size"  ,100,0,100, event.tigress->fCoreHits.size());
      Histogramer::Fill("EventProcessing","prompt: Si.size"    ,100,0,100, event.emma->Si().size());
      Histogramer::Fill("EventProcessing","prompt: IC1.size"   ,100,0,100, event.emma->IC1().size());
      Histogramer::Fill("EventProcessing","prompt: IC2.size"   ,100,0,100, event.emma->IC2().size());
      Histogramer::Fill("EventProcessing","prompt: IC3.size"   ,100,0,100, event.emma->IC3().size());
      Histogramer::Fill("EventProcessing","prompt: IC4.size"   ,100,0,100, event.emma->IC4().size());
      Histogramer::Fill("EventProcessing","prompt: pgac_l.size",100,0,100, event.emma->Left().size());
      Histogramer::Fill("EventProcessing","prompt: pgac_r.size",100,0,100, event.emma->Right().size());
      Histogramer::Fill("EventProcessing","prompt: anode.size" ,100,0,100, event.emma->Anodes().size());
    } // if prompt over
    else{
      Histogramer::Fill("EventProcessing","delay: Core.size"  ,100,0,100, event.tigress->fCoreHits.size());
      Histogramer::Fill("EventProcessing","delay: Si.size"    ,100,0,100, event.emma->Si().size());
      Histogramer::Fill("EventProcessing","delay: IC1.size"   ,100,0,100, event.emma->IC1().size());
      Histogramer::Fill("EventProcessing","delay: IC2.size"   ,100,0,100, event.emma->IC2().size());
      Histogramer::Fill("EventProcessing","delay: IC3.size"   ,100,0,100, event.emma->IC3().size());
      Histogramer::Fill("EventProcessing","delay: IC4.size"   ,100,0,100, event.emma->IC4().size());
      Histogramer::Fill("EventProcessing","delay: pgac_l.size",100,0,100, event.emma->Left().size());
      Histogramer::Fill("EventProcessing","delay: pgac_r.size",100,0,100, event.emma->Right().size());
      Histogramer::Fill("EventProcessing","delay: anode.size" ,100,0,100, event.emma->Anodes().size());
    }

  }
}





