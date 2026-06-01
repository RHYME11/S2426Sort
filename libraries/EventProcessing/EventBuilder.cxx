
#include<EventBuilder.h>
#include<Histogramer.h>

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
  std::lock_guard<std::mutex> lk(fMutex);
  fQueue.push(std::move(frag));
  fPushed++;
}

bool EventBuilder::pop(std::vector<std::unique_ptr<Fragment> > &Builtfrags) {
  std::lock_guard<std::mutex> lk(fMutex);


  if(fQueue.empty()) return false;

  if(fQueue.size()<20000000) {
    if(!fStop) {
        return false;
    }
  }
  {
    auto& top_ref = const_cast<std::unique_ptr<Fragment>&>(fQueue.top());
    Builtfrags.emplace_back(std::move(top_ref));
    fQueue.pop(); 
  }

  long firstTime = Builtfrags.at(0).get()->TimestampNs(); 
  long topTime = -1;
  while(1) {   //currently this never ends...?
    if(fQueue.empty()) {
      break;
    }
    topTime = fQueue.top().get()->TimestampNs();
    if((topTime - firstTime)>2500) {  
      break;
    }
    auto& top_ref = const_cast<std::unique_ptr<Fragment>&>(fQueue.top());
    Builtfrags.emplace_back(std::move(top_ref));
    fQueue.pop();
  }
  //printf("i am here d\n");
  fPopped++;
  return true;
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
