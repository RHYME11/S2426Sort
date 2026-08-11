
#include<EventProcess.h>

#include<EventBuilder.h>
#include<Histogramer.h>
#include<OutputManager.h>

#include<globals.h>


#include <map>
#include <algorithm>
#include <utility>

namespace {

constexpr int kEmtAddress = 0x140f;

}  // namespace


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

    for(auto& frag : builtfrags) {
      if(!frag) continue;

      const int detType = frag->DetType();
      Histogramer::Fill("DetectorType",100,0,100,detType);

      if(detType == 8 && frag->Address() == kEmtAddress) {
        event.timestampNs = frag->TimestampNs();
        continue;
      }

      switch(detType){
        case 0: // TIGRESS core
          event.tigress.fCoreHits.emplace_back(*frag);
          break;
        case 2: // TIGRESS segments
          event.tigress.fSegmentHits.emplace_back(*frag);
          break;
        case 3: // TIGRESS BGO
          event.tigress.fBGOHits.emplace_back(*frag);
          break;
        case 13: // EMMA ADC
          event.emma.AddADC(*frag);
          break;
        case 14: // EMMA TDC
          event.emma.AddTDC(*frag);
          break;
        default:
          break;

      };
    }
 
    event.tigress.BuildHits();
    event.emma.BuildHits();

    OutputManager::Get()->FillEvent(event);

    push(std::move(event));
  }
}
