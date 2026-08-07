
#include<EventBuilder.h>
#include<Histogramer.h>
#include <globals.h>
#include <climits>
#include <set>
#include <utility>
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
// Purpose: Remove exact batch-local duplicates and atomically publish fragments.
// Inputs: Fragments decoded from one MIDAS event.
// Outputs: Unique fragments inserted into fQueue and EMT timestamps into fEMTMap.
void EventBuilder::pushBatch(std::vector<std::unique_ptr<Fragment>> fragments) {
  if(fragments.empty()) {
    return;
  }

  using DuplicateKey = std::pair<int, long>;

  std::set<DuplicateKey> seen;
  std::vector<std::unique_ptr<Fragment>> uniqueFragments;
  uniqueFragments.reserve(fragments.size());

  for(auto& frag : fragments) {
    if(!frag) {
      continue;
    }

    const int number = frag->Number();
    const bool duplicateCandidate = number < 720 || number == 849;

    if(duplicateCandidate) {
      const DuplicateKey key{frag->Address(), frag->TimestampNs()};
      const bool firstOccurrence = seen.emplace(key).second;
      if(!firstOccurrence) {
        continue;
      }
    }

    uniqueFragments.emplace_back(std::move(frag));
  }

  if(uniqueFragments.empty()) {
    return;
  }

  std::lock_guard<std::mutex> lock(fMutex);

  for(auto& frag : uniqueFragments) {
    if(!frag) {
      continue;
    }

    const long ts = frag->TimestampNs();
    if(ts > fLatestTimestampNsSeen) {
      fLatestTimestampNsSeen = ts;
    }
    if(frag->DetType() == 8 && fEMTMap.find(ts) == fEMTMap.end()) {
      fEMTMap.emplace(ts, frag.get());
    }
    fQueue.emplace(ts, std::move(frag));
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
    // ==== BEGIN ==== //
    if(it->second.get()->Number()>849 && it->second.get()->Number()<874) {
      Histogramer::Fill("EventBuilder","dt = EMA - EMTts", 300,-1500,1500,thisTime - EMTts);
    }
    // ===== END ===== //
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
