
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
    // ================================ //
    if(fADCts>0 && fEMMAT>0) Histogramer::Get()->Fill("EventBuilder","dtns_ADC_EMT",2e6,-1e9,1e9,double(fADCts-fEMMAT));
    if(fEMMAT>0 && fTIGts>0) Histogramer::Get()->Fill("EventBuilder","dtns_EMT_TIG",2e5,-1e8,1e8,double(fEMMAT-fTIGts),250,1000,1500,fE);
    if(fADCts>0 && fTIGts>0) Histogramer::Get()->Fill("EventBuilder","dtns_ADC_TIG",2e5,-1e8,1e8,double(fADCts-fTIGts),250,1000,1500,fE);
    if(top_ref.get()->Number()<720 && (top_ref.get()->Number())%15==9) {
      if(top_ref.get()->Energy()>=20 && top_ref.get()->Energy()<=4000) {
        fTIGts = top_ref.get()->TimestampNs();
        fE = top_ref.get()->Doppler(0.0565);
      }
    }
    if(top_ref.get()->Number()==849) fEMMAT = top_ref.get()->TimestampNs();
    if(top_ref.get()->DetType()==13 && (top_ref.get()->Address()&0xff)==16) {
      if(fADCts>0) Histogramer::Get()->Fill("EventBuilder","dtns_ADC",2e3,1e-4,1e-4,top_ref.get()->TimestampNs()-fADCts);
      fADCts = top_ref.get()->TimestampNs();
    }
    // ================================ //
    Builtfrags.emplace_back(std::move(top_ref));
    fQueue.pop(); 
  }

  //printf("i am here c\n");
  long firstTime = Builtfrags.at(0).get()->TimestampNs(); 
  long topTime = -1;
  while(1) {   //currently this never ends...?
    if(fQueue.empty()) {
      break;
    }
    topTime = fQueue.top().get()->TimestampNs();
    if(abs(firstTime - topTime)>2500) {  
      break;
    }
    auto& top_ref = const_cast<std::unique_ptr<Fragment>&>(fQueue.top());
    // ================================ //
    if(fADCts>0 && fEMMAT>0) Histogramer::Get()->Fill("EventBuilder","dtns_ADC_EMT",2e6,-1e9,1e9,double(fADCts-fEMMAT));
    if(fEMMAT>0 && fTIGts>0) Histogramer::Get()->Fill("EventBuilder","dtns_EMT_TIG",2e5,-1e8,1e8,double(fEMMAT-fTIGts),250,1000,1500,fE);
    if(fADCts>0 && fTIGts>0) Histogramer::Get()->Fill("EventBuilder","dtns_ADC_TIG",2e5,-1e8,1e8,double(fADCts-fTIGts),250,1000,1500,fE);
    if(top_ref.get()->Number()<720 && (top_ref.get()->Number())%15==9) {
      if(top_ref.get()->Energy()>=20 && top_ref.get()->Energy()<=4000) {
        fTIGts = top_ref.get()->TimestampNs();
        fE = top_ref.get()->Doppler(0.0565);
      }
    }
    if(top_ref.get()->Number()==849) fEMMAT = top_ref.get()->TimestampNs();
    if(top_ref.get()->DetType()==13 && (top_ref.get()->Address()&0xff)==16) {
      if(fADCts>0) Histogramer::Get()->Fill("EventBuilder","dtns_ADC",2e3,1e-4,1e-4,top_ref.get()->TimestampNs()-fADCts);
      fADCts = top_ref.get()->TimestampNs();
    }
    // ================================ //
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
