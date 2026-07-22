
#include<EventBuilder.h>
#include<Histogramer.h>
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

// ============== pushBatch ==============
// Purpose: Atomically submit fragments from one complete MIDAS event.
// Inputs: Fragments decoded from one MIDAS event.
// Outputs: Fragments inserted into the timestamp-ordered queue.
void EventBuilder::pushBatch(std::vector<std::unique_ptr<Fragment>> fragments) {
  if(fragments.empty()) return;
  std::lock_guard<std::mutex> lk(fMutex);

  for(auto& frag : fragments) {
    if(!frag) continue;

    const long ts = frag->TimestampNs();
    if(ts > fLatestTimestampNsSeen) fLatestTimestampNsSeen = ts;

    fQueue.emplace(ts,std::move(frag));
    fPushed++;
  }


}


bool EventBuilder::pop(std::vector<std::unique_ptr<Fragment>>& Builtfrags) {
  std::lock_guard<std::mutex> lk(fMutex);

  if(fQueue.empty()) return false;

  const long firstTime = fQueue.begin()->first;

  if(!fFlushing) {
    const long safeTime = fLatestTimestampNsSeen - BUILD_WINDOW_NS - REORDER_SLACK_NS;

    if(firstTime > safeTime) {
      return false;
    }
  }

  auto it = fQueue.begin();
  while(it != fQueue.end()) {
    const long thisTime = it->first;

    if(std::labs(thisTime - firstTime) > BUILD_WINDOW_NS) {
      break;
    }
// =================== begin ===================== //
    int number = it->second.get()->Number();
    if(number == 849){
      if(it->second.get()->Energy()<2000){
        fLastEMTTimestampNs = it->second.get()->TimestampNs();
      }
    }
    if(it->second.get()->DetType() == 0){
      fLastCoreTimestampNs = it->second.get()->TimestampNs();
    }
    if(it->second.get()->DetType()==14){
      fLastAnodeTimestampNs = it->second.get()->TimestampNs();
    }
    if(fLastEMTTimestampNs>0 && fLastCoreTimestampNs>0){
      Histogramer::Fill("Fragments","tdif: EMT - Core",5000,0,5000,     fLastEMTTimestampNs/pow(10,9), 
                                                           1000,-5000,5000, fLastEMTTimestampNs-fLastCoreTimestampNs);
    }
    if(fLastEMTTimestampNs>0 && fLastAnodeTimestampNs>0){
      Histogramer::Fill("Fragments","tdif: EMT - Anode",5000,0,5000,     fLastEMTTimestampNs/pow(10,9), 
                                                            1000,-5000,5000, fLastEMTTimestampNs-fLastAnodeTimestampNs);
    }
    if(fLastEMTTimestampNs>0 && fLastCoreTimestampNs>0 && fLastAnodeTimestampNs >0 ){
      Histogramer::Fill("Fragments","tdif: Anode - Core",5000,0,5000,     fLastEMTTimestampNs/pow(10,9), 
                                                             1000,-5000,5000, fLastAnodeTimestampNs-fLastCoreTimestampNs);
    }
// ==================== end ===================== //
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
