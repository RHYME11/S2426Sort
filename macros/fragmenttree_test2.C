{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
  int count0, count1, count2, count3; 
  std::vector<Fragment> core;
  std::vector<Fragment> emt;
  std::vector<Fragment> others;
  
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->DetType()==8) emt.push_back(*ftg);
    if(ftg->DetType()==0) others.push_back(*ftg);
    if(ftg->DetType()==14) others.push_back(*ftg);
  }
 
  std::map<double, std::vector<Fragment>> prompt;
  for(int i=0;i<emt.size();i++) {
    double t = emt[i].TimestampNs();
    prompt[t] = {};
  }
  std::vector<std::vector<Fragment>> delay; 
  int j=0;
  for(int i=0;i<emt.size();i++){
    double reft = emt[i].TimestampNs();
    std::vector<Fragment> tmp_delay;
    while(j<others.size() && others[j].TimestampNs()-reft<-1500){
      tmp_delay.push_back(others[j++]);
    }
    if(!tmp_delay.empty()) delay.push_back(tmp_delay);
    while(j<others.size() && others[j].TimestampNs()-reft<=1500){
      prompt[reft].push_back(others[j++]);
    }
  }
  if(j<others.size()) delay.emplace_back(others.begin()+j,others.end());
 
  TH1D *hs = new TH1D("hs", "single gamma", 8e3,0,8e3); 
  std::vector<double> ve;
  for(auto it=prompt.begin();it!=prompt.end();it++){
    ve.clear();
    bool anode_flag = false;
    bool left_flag = false;
    bool right_flag = false;
    for(int i=0;i<it->second.size();i++){
      Fragment frag = it->second[i];
      if(frag.DetType()==14){
        const int channel = frag.Address() & 0xff;
        if(channel >= 0 && channel <= 2) {anode_flag = true;}
        else if(channel==3) {left_flag = true;}
        else if(channel==4) {right_flag = true;}
      }
      if(frag.DetType()==0 && frag.KValue()==379){ve.push_back(frag.Energy());}
    }
    if(anode_flag && left_flag && right_flag) {
      for(double e:ve) hs->Fill(e);
    }
  } 

  new TCanvas;
  hs->Draw();

}
