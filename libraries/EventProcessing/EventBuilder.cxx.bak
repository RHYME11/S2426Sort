
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
    int number = it->second.get()->Number();
// ============ Duplicate hit clean (begin) =========== //
    if(number<720 || number==849){
      if(duplicate_map.find(number)!=duplicate_map.end()){
        if((thisTime-duplicate_map[number])<=1000){ // time difference < 1us
          it = fQueue.erase(it);
          continue;
        }
      }
      duplicate_map[number] = thisTime;
    }
// ============ Duplicate hit clean (end) =========== //

// =================== begin ===================== // 
    if(number == 849){
      if(it->second.get()->Energy()<2000){
        if(map[0]>0){
          Histogramer::Fill("Fragments","tdif: EMT - Core",5000,0,5000,     map[0]/pow(10,9), 
                                                           1000,-5000,5000, thisTime - map[0]);
        }
        if(map[14]>0){
          Histogramer::Fill("Fragments","tdif: EMT - Anode",5000,0,5000,     map[14]/pow(10,9), 
                                                            1000,-5000,5000, thisTime - map[14]);
        }
        if(map.find(849)!=map.end()){
          Histogramer::Fill("Fragments","tdif: EMT2 - EMT1", 2000,0,2000, map[849]/pow(10,9),
                                                             1000,0,1000, (thisTime - map[849])/pow(10,3));
        }
        map[849] = thisTime;
      }
    } // EMT 
    if(it->second.get()->DetType() == 0){
      if(map[849]>0){
        Histogramer::Fill("Fragments","tdif: EMT - Core",5000,0,5000,     map[849]/pow(10,9), 
                                                         1000,-5000,5000, map[849] - thisTime);
      }
      if(map[14]>0){
        Histogramer::Fill("Fragments","tdif: Anode - Core",5000,0,5000,     map[14]/pow(10,9), 
                                                           1000,-5000,5000, map[14] - thisTime);
      }
      if(map.find(0)!=map.end()){
        Histogramer::Fill("Fragments","tdif: Core2 - Core1",5000,0,5000,    map[0]/pow(10,9),
                                                            1000,-5000,5000,thisTime - map[0]);
      }
      map[0] = thisTime;
    }// Core
    if(it->second.get()->DetType()==14){
      if(map[14]>0 && map[14]!=thisTime){
        Histogramer::Fill("Fragments","tdif: Anode2 - Anode1", 5000,0,5000, map[14]/pow(10,9),                      
                                                               1000,0,1000, (it->second.get()->TimestampNs()-map[14])/pow(10,3));
      }
      if(map[0]>0){
        Histogramer::Fill("Fragments","tdif: Anode - Core",5000,0,5000,     map[0]/pow(10,9), 
                                                           1000,-5000,5000, thisTime-map[0]);
      }
      if(map[849]>0){
        Histogramer::Fill("Fragments","tdif: EMT - Anode",5000,0,5000,     map[849]/pow(10,9), 
                                                          1000,-5000,5000, map[849] - thisTime);
      }
      map[14] = it->second.get()->TimestampNs();
    } // Anode
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
