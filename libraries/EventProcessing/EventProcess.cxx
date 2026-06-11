
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
}

bool EventProcess::pop(DetectorEvent& event) {
  std::lock_guard<std::mutex> lk(fMutex);
  if(fQueue.empty()) return false;

  event = std::move(fQueue.front());
  fQueue.pop();
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
    event.timestamp = builtfrags.front()->Timestamp();
    event.tigress   = std::make_unique<Tigress>();
    event.emma      = std::make_unique<Emma>();
    //event.tip       = std::make_unique<Tip>();


    for(auto& frag : builtfrags) {
      if(!frag) continue;
      Histogramer::Fill("DetectorType",100,0,100,frag->DetType());
      switch(frag->DetType()){
        case 0: // TIGRESS core
          //printf(DYELLOW "TIGRESS %lu " RESET_COLOR "\n",
          //       builtfrags.at(i)->Timestamp());
          //cores.push_back(std::move(builtfrags.at(i)));
          event.tigress->fCoreHits.emplace_back(*frag);
          break;
        case 12: // EMMA ADC
          //printf(DBLUE "EMMA_ADC %lu " RESET_COLOR "\n",
          //       builtfrags.at(i)->Timestamp());
          event.emma->fADC.emplace_back(*frag);
          break;

        case 13: // EMMA TDC
          //printf(DRED "EMMA_TDC %lu " RESET_COLOR "\n",
          //       builtfrags.at(i)->Timestamp());
          event.emma->fTDC.emplace_back(*frag);
          break;

        default:
          break;

      };
    }
    
    event.tigress->BuildHits();
    event.emma->BuildHits();
    //event.tip->BuildHits();

    push(std::move(event));
  }
}

