
#include<EventProcess.h>

#include<EventBuilder.h>
#include<Histogramer.h>

#include<globals.h>


#include <map>
#include <algorithm>
#include <utility>


EventProcess *EventProcess::fEventProcess = 0;

EventProcess::EventProcess() {
  fWorker = std::thread([this]{ this->loop(); });
  fWorker.detach();
}

EventProcess *EventProcess::Get() {
  if(!fEventProcess)
    fEventProcess = new EventProcess;
  return fEventProcess;
}

EventProcess::~EventProcess() { 
  printf("EventProcess destructor called; fStop[%i]\n",fStop.load());
  if(!fStop)
    fWorker.join();
}



void EventProcess::push(DetectorEvent event) {
  std::lock_guard<std::mutex> lk(fMutex);
  fQueue.push(std::move(event));
  fPushed++;
}

bool EventProcess::pop(DetectorEvent& event) {
  std::lock_guard<std::mutex> lk(fMutex);
  if(fQueue.empty()) return false;

  event = std::move(fQueue.front());
  fQueue.pop();
  fPopped++;
  return true;
}



void EventProcess::loop() {
  while(1) {
    if(fStop) break;

    std::vector<std::unique_ptr<Fragment>> builtfrags;

    if(!EventBuilder::Get()->Running()) return;
    if(!EventBuilder::Get()->pop(builtfrags)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));  
      continue;
    }
    if(builtfrags.empty()) continue;

    DetectorEvent event;
    event.timestamp     = builtfrags.front()->Timestamp();
    event.timestampNs   = builtfrags.front()->TimestampNs();
    event.tigress       = std::make_unique<Tigress>();
    event.emma          = std::make_unique<Emma>();
    // =====  begin () ======== //
    long lasttime = builtfrags.back()->TimestampNs();
    // ====== end() 

    for(auto& frag : builtfrags) {
      if(!frag) continue;
      Histogramer::Fill("DetectorType",100,0,100,frag->DetType());
      switch(frag->DetType()){
        case 0: // TIGRESS core
          event.tigress->fCoreHits.emplace_back(*frag);
          break;
        case 8: // EMT
          event.prompt = true;
          break;
        case 13: // EMMA ADC
          event.emma->AddADC(*frag);
          break;
        case 14: // EMMA TDC
          event.emma->AddTDC(*frag);
          break;
        default:
          break;

      };
    }
    // ======== begin() ========== //
    if(event.prompt) Histogramer::Fill("Events","prompt: event time length[ns]",1e7,0,1e8,lasttime - event.timestampNs);
    else             Histogramer::Fill("Events","delay:  event time length[ns]",1e7,0,1e8,lasttime - event.timestampNs);
    // ========= end() =========== //   
 
    event.tigress->BuildHits();
    event.emma->BuildHits();

    push(std::move(event));
  }
}
