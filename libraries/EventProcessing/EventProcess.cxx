
#include<EventProcess.h>

#include<Event.h>
#include<EventBuilder.h>
#include<Histogramer.h>
#include<OutputManager.h>

#include<globals.h>

EventProcess *EventProcess::fEventProcess = 0;

EventProcess::EventProcess() {
  fWorker = std::thread([this]{ this->loop(); });
  fWorker.detach();
}

EventProcess *EventProcess::Get() {
  if(!fEventProcess)
    fEventProcess = new EventProcess;
  return fEventProcess;
}

EventProcess::~EventProcess() { 
  printf("EventProcess destructor called; fStop[%i]\n",fStop.load());
  if(!fStop)
    fWorker.join();
}

void EventProcess::loop() {
  while(1) {
    if(fStop) break;
    
    std::vector<std::unique_ptr<Fragment> > builtfrags;
    if(!EventBuilder::Get()->Running() && EventBuilder::Get()->Size() == 0) {
      fStop = true;
      return;
    }
    if(!EventBuilder::Get()->pop(builtfrags)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));  
      continue;
    }
    fPushed += builtfrags.size(); 
// =============================================================== //
    std::vector<std::unique_ptr<Fragment> >                cores;
    std::map<int,std::vector<std::unique_ptr<Fragment> > > segments;
    std::map<int,std::vector<std::unique_ptr<Fragment> > > suppressors;
    std::vector<std::unique_ptr<Fragment> >                emmat;
    std::vector<std::unique_ptr<Fragment> >                tip;
    std::vector<std::unique_ptr<Fragment> >                emmaadc;
    std::vector<std::unique_ptr<Fragment> >                emmatdc;
    for(size_t i=0;i<builtfrags.size();i++){
      switch(builtfrags.at(i).get()->DetType()) {
        case 0: // tigress core
          cores.push_back(std::move(builtfrags.at(i)));
          break;
        case 1: // likely tigress core (nid)
          //printf("\n\n\ntig core B triggered @ CH%i\n",builtfrags.at(i).get()->Number());
          break;
        case 2: //tigress segment
          break;
        case 3: //tigress suppressor
          break;
        case 8: // EMMA master trigger
          emmat.push_back(std::move(builtfrags.at(i)));
          break;
        case 12: //tip (based on ODB)
          break;
        case 13: // EMMA ADC
          emmaadc.push_back(std::move(builtfrags.at(i)));
          break;
        case 14: // EMMA TDC
          emmatdc.push_back(std::move(builtfrags.at(i)));
          break;
        default:
          break;
      }
    }

    // =================== EMMA ADC ===========================//
    std::vector<long> sits;
    for(auto it = emmaadc.begin(); it!=emmaadc.end(); ++it){
      auto& current = *it;
      int c     = current->Address()&0xff;
      float chg = current->Charge();
      long timestamp = current->Timestamp(); 
      long tsns      = current->TimestampNs();
      if(c == 3) {
        sits.push_back(tsns);
      }
    }

    // =================== EMMA TDC ===========================//
    std::vector<long> anodets;
    for(auto it = emmatdc.begin(); it!=emmatdc.end(); ++it){
      auto& current = *it;
      int c     = current->Address()&0xff;
      float chg = current->Charge();
      long timestamp = current->Timestamp(); 
      long tsns      = current->TimestampNs();
      if(c < 3) {
        anodets.push_back(tsns);
      }
    }
    
    // =================== TIGRES cores ===========================//
    std::vector<long> corets;
    std::vector<double> coreE;
    for(auto it = cores.begin(); it != cores.end(); ++it) {
      auto& current = *it;
      long tsns      = current->TimestampNs();
      corets.push_back(tsns);
      coreE.push_back(current->Energy());
    } 
    
    Histogramer::Get()->Fill("Event","Si_size",10,0,10,sits.size());
    for(int i=0;i<sits.size();i++){
      for(int j=0;j<anodets.size();j++){
        Histogramer::Get()->Fill("Event","adc_tsns - tdc_tsns",1000,-5000,5000,sits[i] - anodets[j]);
      }// anode loop j over
      for(int j=0;j<corets.size();j++){
        long tdif = sits[i] - corets[j];
        //Histogramer::Get()->Fill("Event","adc_tsns - core_tsns vs coreE",500,-2500,2500,tdif, 4e3,0,4e3,coreE[j]);
        Histogramer::Get()->Fill("Event","adc_tsns - core_tsns",1000,-5000,5000,tdif);
      }// core loop j over
    }// si loop i over
// =============================================================== //
    std::vector<Fragment> fragments;
    fragments.reserve(builtfrags.size());
    for(const auto& frag : builtfrags) {
      if(frag) {
        OutputManager::Get()->FillFragment(*frag);
        fragments.push_back(*frag);
      }
    }

    Event event;
    event.Set(fragments);
    OutputManager::Get()->FillEvent(event);
    if(event.IsGood()) OutputManager::Get()->FillGoodEvent(event);

  }
  fStop = true;
};
