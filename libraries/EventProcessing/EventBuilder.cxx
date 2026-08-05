
#include<EventBuilder.h>
#include<Histogramer.h>
#include <globals.h>
#include <climits>
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
    if(frag->DetType()==8 && fEMTMap.find(ts)==fEMTMap.end()) fEMTMap.emplace(ts,frag.get());
    fQueue.emplace(ts,std::move(frag));
    fPushed++;
  }
}


bool EventBuilder::pop(std::vector<std::unique_ptr<Fragment>>& Builtfrags) {
  std::lock_guard<std::mutex> lk(fMutex);

  if(fQueue.empty()) return false;
  const long firstTime = fQueue.begin()->first;

  if(!fFlushing) {
    if(fEMTMap.empty()) return false;
    const long safeTime = fLatestTimestampNsSeen - REORDER_SLACK_NS;
    if(firstTime > safeTime) {
      return false;
    }
  }

  long EMTts = -1;
  if(!fEMTMap.empty()) EMTts = fEMTMap.begin()->first;
  bool buildingbg = false;
  bool buildingprompt = false;
  auto it = fQueue.begin();
  while(it!=fQueue.end()){
    const long thisTime = it->first;
    // ============ Duplicate hit clean (begin) =========== //
    int number = it->second.get()->Number();
    if(number<720 || number==849){
      if(duplicate_map.find(number)!=duplicate_map.end()){
        if((thisTime-duplicate_map[number])<=DUPLICATE_WINDOW_NS){ // time difference < 1us
          it = fQueue.erase(it);
          continue;
        }
      }
      duplicate_map[number] = thisTime;
    }
    // ============ Duplicate hit clean (end) =========== //
    printf("fQueue = %lu\t EMTts = %lu\n", thisTime, EMTts);
    if(EMTts<0){ // fFLushing must be true
      Builtfrags.emplace_back(std::move(it->second));
      it = fQueue.erase(it);  
    }
    if(thisTime - EMTts < -1500){ // background events
      Builtfrags.emplace_back(std::move(it->second));
      it = fQueue.erase(it);  
      buildingbg = true;
      continue;
    }
    if(buildingbg){
      break;
    }
    if((thisTime - EMTts>=-1500) && (thisTime - EMTts<=1500)){ // prompt events
      Builtfrags.emplace_back(std::move(it->second));
      it = fQueue.erase(it);  
      buildingprompt = true;
      continue;
    } 
    if(buildingprompt){
      fEMTMap.erase(fEMTMap.begin());
      break;
    }
  } // loop fQueue over
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
