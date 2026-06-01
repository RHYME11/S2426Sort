
#include<EventProcess.h>

#include<Event.h>
#include<EventBuilder.h>
#include<Histogramer.h>
#include<OutputManager.h>

#include<globals.h>

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

void EventProcess::loop() {
  while(1) {
    if(fStop) break;
    
    std::vector<std::unique_ptr<Fragment> > builtfrags;
    if(!EventBuilder::Get()->Running() && EventBuilder::Get()->Size() == 0) {
      fStop = true;
      return;
    }
    if(!EventBuilder::Get()->pop(builtfrags)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));  
      continue;
    }
    fPushed += builtfrags.size(); 

    std::vector<Fragment> fragments;
    fragments.reserve(builtfrags.size());
    for(const auto& frag : builtfrags) {
      if(frag) {
        OutputManager::Get()->FillFragment(*frag);
        fragments.push_back(*frag);
      }
    }

    Event event;
    event.Set(fragments);
    OutputManager::Get()->FillEvent(event);
    if(event.IsGood()) OutputManager::Get()->FillGoodEvent(event);

  }
  fStop = true;
};
