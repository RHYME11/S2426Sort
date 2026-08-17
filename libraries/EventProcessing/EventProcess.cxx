
#include<EventProcess.h>

#include<EventBuilder.h>
#include<OutputManager.h>

#include<globals.h>


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

    std::vector<Fragment> fragments;
    fragments.reserve(builtfrags.size());
    long referenceTimestampNs = -1;

    for(const auto& frag : builtfrags) {
      if(!frag) continue;

      const int detType = frag->DetType();
      const int channel = frag->Address() & 0xff;
      if(referenceTimestampNs < 0 && detType == 14 && channel >= 0 && channel <= 2) {
        referenceTimestampNs = frag->TimestampNs();
      }

      fragments.emplace_back(*frag);
    }

    DetectorEvent event(std::move(fragments), referenceTimestampNs);

    OutputManager::Get()->FillEvent(event);

    push(std::move(event));
  }
}
