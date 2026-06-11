
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

//bool EventBuilder::push(Fragment *frag) {
//  std::lock_guard<std::mutex> lk(fMutex);
//  if(fQueue.size()>500000) return false;

//  fQueue.push(std::move(frag)); 
//  return true;
//}
/*
bool EventBuilder::pop(std::vector<Fragment*> &BuiltFrags) {
  std::lock_guard<std::mutex> lk(fMutex);
  if(fQueue.empty()) return false;

  Fragment *first = fQueue.top();
  BuiltFrags.push_back(first);
  fQueue.pop();
  //one in vector - now fill vector till build window.
  if(fQueue.empty()) return true; // at least one already in the vector. 
  return true;
  Fragment *other = fQueue.top();
  while(abs(other->Timestamp() - first->Timestamp())<10000) { 
    BuiltFrags.push_back(other);
    fQueue.pop();
    other = fQueue.top();
  }

  return true;
}
*/


void EventBuilder::push(std::unique_ptr<Fragment> frag) {
  if(!frag) return;
  std::lock_guard<std::mutex> lk(fMutex);
 
  const long ts = frag->Timestamp();
  if(ts> fLatestTimestampSeen) fLatestTimestampSeen = ts;

  fQueue.emplace(ts,std::move(frag));
  fPushed++;
}


bool EventBuilder::pop(std::vector<std::unique_ptr<Fragment>>& Builtfrags) {
  std::lock_guard<std::mutex> lk(fMutex);

  if(fQueue.empty()) return false;

  //if(fQueue.size() < 1000000 && !fStop) {
  //  return false;
  //}

  const long firstTime = fQueue.begin()->first;

if(!fFlushing) {
    const long safeTime = fLatestTimestampSeen - BUILD_WINDOW - REORDER_SLACK;

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



/*
bool EventBuilder::peek(Fragment*& out) {
  std::lock_guard<std::mutex> lk(fMutex);
  if(fQueue.empty()) {
    out = nullptr;
    return false;
  }
  out = fQueue.top().get();
  return true; 
}
*/

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
