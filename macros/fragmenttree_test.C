{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
  
  TH2D *hcore= new TH2D("hcore","Core - Anode [ns] vs exptime",2e3,0,2e3,2e4,-1e5,1e5);
  TH2D *hseg = new TH2D("hseg ","Seg  - Anode [ns] vs exptime",2e3,0,2e3,2e4,-1e5,1e5);
  TH2D *hbgo = new TH2D("hbgo ","BGO  - Anode [ns] vs exptime",2e3,0,2e3,2e4,-1e5,1e5);
  std::vector<long> core_vec;
  std::vector<long> and_vec;
  std::vector<long> seg_vec;
  std::vector<long> bgo_vec;
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    long thisTime = ftg->TimestampNs();
    if(ftg->DetType()==0) core_vec.push_back(thisTime);
    if(ftg->DetType()==2) seg_vec.push_back(thisTime);
    if(ftg->DetType()==3 && ftg->Charge()>100) bgo_vec.push_back(thisTime);
    if(ftg->Number()>=866 && ftg->Number()<=868) {
      if(and_vec.empty() || and_vec.back()!=thisTime)and_vec.push_back(thisTime);
    }
  }
  int indx_core = 0;
  int indx_seg  = 0;
  int indx_bgo  = 0;
  for (int i = 0; i < and_vec.size(); i++) {
    long thisTime = and_vec[i];
    bool isLast = (i == and_vec.size() - 1);
    long nextTime = 0;
    if (!isLast) nextTime = and_vec[i + 1];
    while (indx_core < core_vec.size()) {
        if (!isLast && core_vec[indx_core] > nextTime) break;
        double dt1 = core_vec[indx_core] - thisTime;
        if (isLast) {
            hcore->Fill(thisTime / 1e9, dt1);
        }else {
            double dt2 = core_vec[indx_core] - nextTime;
            if (fabs(dt1) < fabs(dt2)) hcore->Fill(thisTime / 1e9, dt1);
            else                       hcore->Fill(nextTime / 1e9, dt2);
        }
        indx_core++;
    }// core loop over
    while (indx_seg < seg_vec.size()) {
        if (!isLast && seg_vec[indx_seg] > nextTime) {break;}
        double dt1 = seg_vec[indx_seg] - thisTime;
        if (isLast) { hseg->Fill(thisTime / 1e9, dt1);}
        else {
            double dt2 = seg_vec[indx_seg] - nextTime;
            if (fabs(dt1) < fabs(dt2)) hseg->Fill(thisTime / 1e9, dt1);
            else                       hseg->Fill(nextTime / 1e9, dt2);
        }
        indx_seg++;
    }// seg loop over
    while (indx_bgo < bgo_vec.size()) {
        if (!isLast && bgo_vec[indx_bgo] > nextTime) break;
        double dt1 = bgo_vec[indx_bgo] - thisTime;
        if (isLast) {hbgo->Fill(thisTime / 1e9, dt1);}
        else {
            double dt2 = bgo_vec[indx_bgo] - nextTime;
            if (fabs(dt1) < fabs(dt2)) hbgo->Fill(thisTime / 1e9, dt1);
            else                       hbgo->Fill(nextTime / 1e9, dt2);
        }
        indx_bgo++;
    }// bgo loop over
}
  new TCanvas; hcore->Draw();
  new TCanvas; hseg ->Draw();
  new TCanvas; hbgo ->Draw();


  /*TH1D *hist = new TH1D("hist" ,"BGO - Core from same crystal [ns]",2e4,-1e4,1e4);
  TH2D *hist2= new TH2D("hist2","BGO - Core from same crystal [ns] vs exptime",2e3,0,2e3,2e3,-1e4,1e4);
  std::map<int,double> core_map;
  std::map<int,std::vector<double>> bgo_map;
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    int arrynum = (ftg->Number())/15 + 17;
    long thisTime = ftg->Time();
    if(ftg->DetType()==3) {
      if(ftg->Charge()>100){
        bgo_map[arrynum].push_back(thisTime);
      }
    }
    if(ftg->DetType()==0){
      if(core_map[arrynum]>0){
        for(int i=0;i<bgo_map[arrynum].size();i++){
          double dt1 = bgo_map[arrynum].at(i) - core_map[arrynum];
          double dt2 = bgo_map[arrynum].at(i) - thisTime;
          if(fabs(dt1)<fabs(dt2)) {
            hist->Fill(dt1);
            hist2->Fill(core_map[arrynum]/1e9,dt1);
          }else {
            hist->Fill(dt2);
            hist2->Fill(thisTime/1e9,dt2);
          }
        } // seg loop over
        bgo_map[arrynum].clear();
      }
      core_map[arrynum] = thisTime;
    }
  }
  for(auto it=bgo_map.begin();it!=bgo_map.end();it++){
    int arrynum = it->first;
    if(core_map[arrynum]==0){
      printf("arraynumber = %i/n",arrynum);
      continue;
    }
    for(int i=0;i<it->second.size();i++){
      double dt1 = it->second.at(i) - core_map[arrynum];
      hist->Fill(dt1);                            
      hist2->Fill(core_map[arrynum]/1e9,dt1);
    }
  }

  new TCanvas;hist->Draw();
  new TCanvas;hist2->Draw();
  */  
  /*TH1D *hist = new TH1D("hist" ,"Core1 - Core2 [ns]",1e3,0,1e4);
  TH1D *hist1= new TH1D("hist1","dt Core with same number [ns]",1e3,0,1e4);
  std::pair<int,long> fLastSeen{-1,0};
  std::map<int,long> core_map;
  int count0=0;
  int count1=0;
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->DetType()!=0) continue;
    long thisTime = ftg->TimestampNs();
    int number = ftg->Number();
    count0++;
    if(core_map[number]==0) core_map[number] = thisTime;
    else{
      hist1->Fill(thisTime-core_map[number]);
      core_map[number] = thisTime;
    }
    if(fLastSeen.second == 0) fLastSeen = make_pair(number,thisTime);
    else{
      double dt = thisTime - fLastSeen.second;
      hist->Fill(dt);
      if(dt<=220 && number==fLastSeen.first) count1++;
      fLastSeen = make_pair(number,thisTime); 
    }
  }
  new TCanvas; hist ->Draw();
  new TCanvas; hist1->Draw();
  */
  /*map<long,vector<Fragment>> emma_map;
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->Number()>857 && ftg->Number()<874){
      emma_map[ftg->TimestampNs()].push_back(*ftg);
    }
  }

  int count0, count1, count2, count3, count4;
  for(auto it=emma_map.begin();it!=emma_map.end();it++){
    bool Si    = false;
    bool IC    = false;
    bool Anode = false;
    bool Left  = false;
    bool Right = false;
    bool Top   = false;
    bool Bot   = false;
    std::set<int> tdcChannels;
    for(int i=0;i<it->second.size();i++){
      const int number = it->second[i].Number();
      switch(number){
        case 861:
          Si = true;
          break;
        case 862 ... 865:
          IC = true;
          break;
        case 866 ... 868:
          tdcChannels.insert(number);
          Anode = true;
          break;
        case 869:
          tdcChannels.insert(number);
          Left = true;
          break;
        case 870:
          tdcChannels.insert(number);
          Right = true;
          break;
        case 871:
          tdcChannels.insert(number);
          Top = true;
          break;
        case 872:
          tdcChannels.insert(number);
          Bot = true;
          break;
        default:
          break;
      }// switch over
    }// vector loop over
    const int tdcsize = static_cast<int>(tdcChannels.size());
    if(Anode) count2++;
    if(Anode && tdcsize>4) {
      count3++;
      if(Si && IC) count4++; 
    }
    if(Anode && Left && Right && Top && Bot){
      count0++;
      if(Si && IC) count1++;
    }
  }

  printf("total EMMA group              = %zu\n", emma_map.size());
  printf("EMMA group (has anode)        = %i\n", count2);
  printf("EMMA group (has anode && tdcsize>4) = %i\n", count3);
  printf("EMMA group (has anode && Si+IC && tdcsize>4) = %i\n", count4);
  printf("EMMA group (has anode && 4 cathodes)         = %i\n", count0); 
  printf("EMMA group (has anode && Si+IC && 4 cathodes)= %i\n", count1); 
  */
}
