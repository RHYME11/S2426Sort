
#include<EventBuilder.h>

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
    //printf("First:%s(%i): %lu\n",top_ref->Name().c_str(),top_ref->DetType(),top_ref->TimestampNs());
    Builtfrags.emplace_back(std::move(top_ref));
    fQueue.pop(); 
  }

  //printf("i am here c\n");
  //long firstTime = Builtfrags.at(0).get()->Timestamp(); 
  long firstTime = Builtfrags.at(0).get()->TimestampNs(); 
  long topTime = -1;
  while(1) {   //currently this never ends...?
    if(fQueue.empty()) {
      break;
    }
    //topTime = fQueue.top().get()->Timestamp();
    topTime = fQueue.top().get()->TimestampNs();
    //printf("\n\n\n\nabs(firstTime - topTime): %lu\n\n\n\n",abs(firstTime - topTime));
    if(abs(firstTime - topTime)>2500) {
      //printf("====BREAK by window: firstTime = %lu, topTime = %ld, diff = %ld, queue size(%d) = %lu====\n\n\n",
      // firstTime, topTime, labs(firstTime - topTime), (int)fStop.load(), fQueue.size());
      break;
    }
    auto& top_ref = const_cast<std::unique_ptr<Fragment>&>(fQueue.top());
    //printf("%s(%i): %lu\n",top_ref->Name().c_str(),top_ref->DetType(),top_ref->TimestampNs());
    Builtfrags.emplace_back(std::move(top_ref));
    fQueue.pop();
  }
  //printf("i am here d\n");
  fPopped++;
  return true;
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
