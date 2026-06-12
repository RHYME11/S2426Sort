
#include<EventBuilder.h>

#include <globals.h>

EventBuilder *EventBuilder::fEventBuilder = 0;

EventBuilder::EventBuilder() {
  fWorker = std::thread([this]{ this->loop(); });
  fWorker.detach();
}

EventBuilder *EventBuilder::Get() {
  if(!fEventBuilder)
    fEventBuilder = new EventBuilder;
  return fEventBuilder;
}

EventBuilder::~EventBuilder() { 
  printf("EventBuilder destructor called; fStop[%i]\n",fStop.load());

  if(!fStop)
    fWorker.join();
}

void EventBuilder::push(std::unique_ptr<Fragment> frag) {
  if(!frag) return;
  std::lock_guard<std::mutex> lk(fMutex);
 
  const long ts = frag->TimestampNs();
  if(ts> fLatestTimestampNsSeen) fLatestTimestampNsSeen = ts;

  fQueue.emplace(ts,std::move(frag));
  fPushed++;
}


bool EventBuilder::pop(std::vector<std::unique_ptr<Fragment>>& Builtfrags) {
  std::lock_guard<std::mutex> lk(fMutex);

  if(fQueue.empty()) return false;

  const long firstTime = fQueue.begin()->first;

if(!fFlushing) {
    const long safeTime = fLatestTimestampNsSeen - BUILD_WINDOW - REORDER_SLACK;

    if(firstTime > safeTime) {
      return false;
    }
  }

  auto it = fQueue.begin();

  while(it != fQueue.end()) {
    const long thisTime = it->first;

    if(std::labs(thisTime - firstTime) > BUILD_WINDOW) {
      break;
    }

    Builtfrags.emplace_back(std::move(it->second));
    it = fQueue.erase(it);
  }

  fPopped++;
  return !Builtfrags.empty();
}


void EventBuilder::loop() {

  while(1) {
    bool doBreak = false;
    {
      std::lock_guard<std::mutex> lk(fMutex);
      if(fStop && fQueue.empty()) doBreak = true;
      //checks que
      // - if true; pass built events;
      // - if flase; sleep;
    }
    if(doBreak) break;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));  
  }
};
