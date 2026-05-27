
#include<EventProcess.h>

#include<EventBuilder.h>
#include<Histogramer.h>

#include<globals.h>

#include <map>
#include <iostream>
#include "TVector3.h"
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

// ======================= //
double last_EMT = -1;
// ======================= //
void EventProcess::loop() {
  long lasttime    = -1;
  long currenttime =  0;
  while(1) {
    if(fStop) break;
    
    //printf("\n\n\n\n EVENT PROCESS LOOP  ?!\n\n\n");

    //printf("here 1\n");
    std::vector<std::unique_ptr<Fragment> > builtfrags;
    if(!EventBuilder::Get()->Running() && EventBuilder::Get()->Size() == 0) return;
    if(!EventBuilder::Get()->pop(builtfrags)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));  
      continue;
    }
    fPushed += builtfrags.size(); 
    //printf("here 2\n");
    //printf("frags: %lu\n",builtfrags.size());      
    //continue;

    //std::map<int,std::vector<std::unique_ptr<Fragment> > > cores;
    std::vector<std::unique_ptr<Fragment> >                cores;
    std::map<int,std::vector<std::unique_ptr<Fragment> > > segments;
    std::map<int,std::vector<std::unique_ptr<Fragment> > > suppressors;
    std::vector<std::unique_ptr<Fragment> >                emmat;
    std::vector<std::unique_ptr<Fragment> >                tip;
    std::vector<std::unique_ptr<Fragment> >                emmaadc;
    std::vector<std::unique_ptr<Fragment> >                emmatdc;
    
    fCounter[0]++;
    for(size_t i=0;i<builtfrags.size();i++){
      //if(i==0) printf("\n\n=======================================================\n");
      //printf("%s(0x%08x): %lu\n",builtfrags.at(i).get()->Name().c_str(), 
      //                           builtfrags.at(i).get()->Address(),
      //                           builtfrags.at(i).get()->TimestampNs());   
 
      //ideal i would move the below code to a "physics-loop" 
      //but this will do for now.
      //do physics...
      switch(builtfrags.at(i).get()->DetType()) {
        case 0: // tigress core
          cores.push_back(std::move(builtfrags.at(i)));
          break;
        case 1: // likely tigress core (nid)
          printf("\n\n\ntig core B triggered @ CH%i\n",builtfrags.at(i).get()->Number());
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
    std::map<int,std::vector<double>> ics;
    std::map<int,std::vector<long>>   icst;
    std::vector<double> si;
    for(auto it = emmaadc.begin(); it!=emmaadc.end(); ++it){
      auto& current = *it;
      int c     = current->Address()&0xff;
      float chg = current->Charge();
      long timestamp = current->Timestamp(); 
      long tsns      = current->TimestampNs();
      //Histogramer::Get()->Fill("Event/EMMA","eADC_event",4000,0,64000,chg, 1000,0,1000,c);
      if(c>15 && c<20) {
        ics[c-16].push_back(chg);
        icst[c-16].push_back(tsns);
      }
      if(c == 3) {
        si.push_back(chg);
      }
    }
    double sum = 0;
		std::map<int, double> ic_sum;
    for(const auto& [ic_id, values] : ics) {
      for(double v : values) {
        ic_sum[ic_id] += v;
        sum += v;
      }
    }
		double sic = 0;
		for(double v : si) {sum += v; sic += v;}
		for(int i = 0; i < 4; ++i) {
      Histogramer::Get()->Fill("Event/EMMA", Form("ic%d_vs_sum",i), 4000, 0, 4000, sum,4000, 0, 4000, ic_sum[i]);
		  Histogramer::Get()->Fill("Event/EMMA", Form("ic%d_vs_si",i),  4000, 0, 4000, sic,4000, 0, 4000, ic_sum[i]);
		  for(int j = i + 1; j < 4; ++j) {
		   	if(ic_sum.count(i) && ic_sum.count(j)) {
		   		Histogramer::Get()->Fill("Event/EMMA",Form("ic%d_vs_ic%d", i, j),4000, 0, 4000, ic_sum[i],4000, 0, 4000, ic_sum[j]);
		   	}
		  }
		} 

// ============================================================//

// =================== EMMA TDC ===========================//
    double fLdelay = 40;
		double fRdelay = 20;
		double fTdelay = 10;
		double fBdelay = 20;
		double fXlength = 80.; // Size of X focal plane in mm
		double fYlength = 30.; // Size of Y focal plane in mm
		double left   = -1;
		double right  = -1;
		double top    = -1;
		double bottom = -1;	
		for(auto it = emmatdc.begin(); it!=emmatdc.end(); ++it){
      auto& current = *it;
      int c     = current->Address()&0xff;
      float chg = current->Charge(); 
      long timestamp = current->Timestamp(); 
      long tsns      = current->TimestampNs(); 
      Histogramer::Get()->Fill("Event/EMMA","eTDC_event",4000,0,64000,chg, 1000,0,1000,c);
			if(c==3) right  = chg;
			if(c==4) left   = chg;
			if(c==5) top    = chg;
			if(c==6) bottom = chg;
    }
		if(left>0 && right>0 && top>0 && bottom>0){
			double Xdiff = (left+fLdelay) - (right+fRdelay);
			double Xsum = (left) + (right);
  		double Ydiff = (bottom+fBdelay) - (top+fTdelay);
  		double Ysum = (bottom) + (top);              
  		double Xpos = ( Xdiff / Xsum )*fXlength;
  		double Ypos = ( Ydiff / Ysum )*fYlength;
			TVector3 pgac(Xpos, Ypos, 1);
			Histogramer::Get()->Fill("Event/PGAC","focalplanx_y", 60,-30,30,pgac.X(),60,-30,30,pgac.Y());
		}
// ============================================================//

// =================== TIGRES cores ===========================//
    std::sort(cores.begin(), cores.end(),
      [](const std::unique_ptr<Fragment>& a,
        const std::unique_ptr<Fragment>& b) {
        return a->Energy() > b->Energy();  // decending order.
    });

    std::vector<double> vec_doppler;
    std::vector<long>   vec_ts;
    for(auto it = cores.begin(); it != cores.end(); ++it) {
      auto& current = *it;
      //std::cout << "frag name = [" << current->Name() << "] [" << current->DetType() << "]" << std::endl;
      if(current->Energy()<20 || current->Energy()>4000) continue;
      int  det   = std::stoi(current->Name().substr(3,2));
      char color = current->Name().at(5);
      int  xtal  = (color == 'B') ? 0 :
                   (color == 'G') ? 1 :
                   (color == 'R') ? 2 :
                   (color == 'W') ? 3 : -1;
      Histogramer::Get()->Fill("Event","summary_energy", 70,0,70,det*4 +xtal,8000,0,4000,current->Energy());
      Histogramer::Get()->Fill("Event","summary_doppler",70,0,70,det*4 +xtal,8000,0,4000,current->Doppler(0.0565));
      //Histogramer::Get()->Fill("Event","summary_charge",70,0,70,det*4 +xtal,16000,0,16000,current->Charge());
      //Histogramer::Get()->Fill(Form("Event/x%02i%c",det,color),"time_charge",7200,0,72000,current->Time()/1e8,16000,0,16000,current->Charge());
      long timestamp = current->Timestamp();
      long tsns      = current->TimestampNs();
      vec_doppler.push_back(current->Doppler(0.0565));
      vec_ts.push_back(tsns);
      //auto next = std::next(it);
      //if(next == cores.end()) break;  // no next element
      //auto& nextCore = *next;
      //if(nextCore->Name().size()<10) continue;
      //Histogramer::Get()->Fill("Event","dtime",6000,-3000,3000,current->Time() - nextCore->Time(),4000,0,4000,nextCore->Energy());
      //Histogramer::Get()->Fill("Event","dtimestamp",4000,-2000,2000,current->Timestamp() - nextCore->Timestamp(),4000,0,4000,nextCore->Energy());
    }
    if(icst[0].size()>0 && vec_ts.size()>0){
      fCounter[1]++;
      for(int i=0;i<icst[0].size();i++){
        for(int j=0;j<vec_ts.size();j++){
          Histogramer::Get()->Fill("Event/Good","dtns_ADC_TIG",2e3,-1e4,1e4,double(icst[0][i]-vec_ts[j]),4e3,0,4e3,vec_doppler[j]);
        }
      }
      Histogramer::Get()->Fill("Event/Good","core_size",100,0,100,vec_ts.size());
      Histogramer::Get()->Fill("Event/Good","IC0_size" ,100,0,100,icst[0].size()); 
      Histogramer::Get()->Fill("Event/Good","IC0x_corey_size" ,100,0,100,icst[0].size(),100,0,100,vec_ts.size()); 
      if(right<0 || left<0 || top<0 || bottom<0) fCounter[2]++;
      if(right<0 || left<0) fCounter[3]++;
	    for(int i = 0; i < 4; ++i) {
        Histogramer::Get()->Fill("Event/Good/EMMA", Form("ic%d_vs_sum",i), 4000, 0, 4000, sum,4000, 0, 4000, ic_sum[i]);
	      Histogramer::Get()->Fill("Event/Good/EMMA", Form("ic%d_vs_si",i),  4000, 0, 4000, sic,4000, 0, 4000, ic_sum[i]);
	      for(int j = i + 1; j < 4; ++j) {
	       	if(ic_sum.count(i) && ic_sum.count(j)) {
	       		Histogramer::Get()->Fill("Event/Good/EMMA",Form("ic%d_vs_ic%d", i, j),4000, 0, 4000, ic_sum[i],4000, 0, 4000, ic_sum[j]);
	       	}
	      }
	    } 
	    if(left>0 && right>0 && top>0 && bottom>0){
	    	double Xdiff = (left+fLdelay) - (right+fRdelay);
	    	double Xsum = (left) + (right);
      	double Ydiff = (bottom+fBdelay) - (top+fTdelay);
      	double Ysum = (bottom) + (top);              
      	double Xpos = ( Xdiff / Xsum )*fXlength;
      	double Ypos = ( Ydiff / Ysum )*fYlength;
	    	TVector3 pgac(Xpos, Ypos, 1);
	    	Histogramer::Get()->Fill("Event/Good/PGAC","focalplanx_y", 60,-30,30,pgac.X(),60,-30,30,pgac.Y());
        for(int j=0;j<vec_doppler.size();j++){
          Histogramer::Get()->Fill("Event/Good/PGAC","pgacx_doppler_allchn",60,-30,30,pgac.X(),4e3,0,4e3,vec_doppler[j]);
        }
	    }
	    if(left>0 && right>0){
	    	double Xdiff = (left+fLdelay) - (right+fRdelay);
	    	double Xsum = (left) + (right);
      	double Xpos = ( Xdiff / Xsum )*fXlength;
        for(int j=0;j<vec_doppler.size();j++){
          Histogramer::Get()->Fill("Event/Good/PGAC","pgacx_doppler",60,-30,30,Xpos,4e3,0,4e3,vec_doppler[j]);
        }
	    }
    } // good event over

    if(icst[0].size()==0 && vec_ts.size()>0){
      for(int j=0;j<vec_ts.size();j++){
        Histogramer::Get()->Fill("Event","single_noemma",4e3,0,4e3,vec_doppler[j]);
      }
    }
      
// ============================================================//


  }
};


// ======================== PrintCounter() ====================================//
void EventProcess::PrintCounter() const {
  printf("\n===== EventProcess Counters =====\n");

  for(int i = 0; i < 100; i++) {
    if(fCounter[i] == 0) continue;
    printf("fCounter[%02d] = %ld\n", i, fCounter[i]);
  }

  printf("=================================\n\n");
}


