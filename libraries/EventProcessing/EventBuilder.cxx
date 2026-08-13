
#include<EventBuilder.h>
#include<Histogramer.h>
#include<OutputManager.h>
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
// Purpose: Remove exact same-batch and adjacent-batch GRF4 duplicates.
// Inputs: Fragments decoded from one MIDAS event.
// Outputs: Unique fragments inserted into fQueue and anode timestamps into fRefMap.
void EventBuilder::pushBatch(std::vector<std::unique_ptr<Fragment>> fragments) {
  if(fragments.empty()) {
    return;
  }

  std::lock_guard<std::mutex> lock(fMutex);
  std::set<std::pair<int, long>> currentBatchKeys;
  std::vector<std::unique_ptr<Fragment>> uniqueFragments;
  uniqueFragments.reserve(fragments.size());

  for(auto& frag : fragments) {
    if(!frag) {
      continue;
    }

    const long ts = frag->TimestampNs();
    if(ts > fLatestTimestampNsSeen) {
      fLatestTimestampNsSeen = ts;
    }

    const int number = frag->Number();
    if(number < 720) {
      const std::pair<int, long> key = std::make_pair(frag->Address(), ts);
      const bool firstInCurrentBatch = currentBatchKeys.emplace(key).second;
      if(!firstInCurrentBatch) {
        continue;
      }
      if(fPreviousBatchKeys.find(key) != fPreviousBatchKeys.end()) {
        continue;
      }
    }

    uniqueFragments.emplace_back(std::move(frag));
  }

  // Keep observed keys even when their fragments matched the previous batch.
  // This also suppresses one hit repeated across three consecutive batches.
  if(!currentBatchKeys.empty()) {
    fPreviousBatchKeys = std::move(currentBatchKeys);
  }

  for(auto& frag : uniqueFragments) {
    if(!frag) {
      continue;
    }

    const long ts = frag->TimestampNs();
    const int channel = frag->Address() & 0xff;
    if(frag->DetType() == 14 && channel >= 0 && channel <= 2 &&
        fRefMap.find(ts) == fRefMap.end()) {
      fRefMap.emplace(ts, frag.get());
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
    if(fRefMap.empty()) return false;
    const long safeTime = fLatestTimestampNsSeen - REORDER_SLACK_NS;
    if(firstTime > safeTime) {
      return false;
    }
  }

  long refTime = -1;
  if(!fRefMap.empty()) refTime = fRefMap.begin()->first;
  bool buildingbg = false;
  bool buildingprompt = false;

  auto moveToBuilt = [&Builtfrags, this](auto current) {
    OutputManager::Get()->FillFragment(*current->second);
    Builtfrags.emplace_back(std::move(current->second));
    return fQueue.erase(current);
  };

  auto it = fQueue.begin();
  while(it!=fQueue.end()){
    const long thisTime = it->first;
    const long dt = thisTime - refTime;
    // ==== BEGIN ==== //
    if(it->second.get()->Number()>849 && it->second.get()->Number()<874) {
      Histogramer::Fill("EventBuilder","dt = EMMA - refTime", 300,-400,2600,dt);
    }
    // ===== END ===== //
    if(refTime<0){ // fFlushing must be true
      it = moveToBuilt(it);
      continue;
    }
    if(dt < BUILD_WINDOW_NS.first){ // background events
      it = moveToBuilt(it);
      buildingbg = true;
      continue;
    }
    if(buildingbg){
      break;
    }
    if(dt >= BUILD_WINDOW_NS.first && dt <= BUILD_WINDOW_NS.second){ // prompt events
      it = moveToBuilt(it);
      buildingprompt = true;
      continue;
    } 
    if(buildingprompt){
      fRefMap.erase(fRefMap.begin());
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
