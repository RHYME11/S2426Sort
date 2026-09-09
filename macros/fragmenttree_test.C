{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
  int count0, count1, count2, count3; 
  std::vector<double> core;
  std::vector<double> emt;
  
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->DetType()==8) emt.push_back(ftg->TimestampNs());
    if(ftg->DetType()==0) core.push_back(ftg->TimestampNs());
  }
 
  std::map<double, std::vector<double>> prompt;
  std::vector<std::vector<double>> delay; 
  std::vector<double> tmp_delay;
  int j=0;
  for(int i=0;i<emt.size();i++){
    double reft0 = emt[i];
    prompt[reft0] = {};
    if(!tmp_delay.empty()){
      delay.push_back(tmp_delay);
    }
    tmp_delay.clear();
    while(j<core.size()){
      double coret = core[j];
      double dt0   = coret - reft0 ;
      if(dt0<-1500){
        tmp_delay.push_back(coret);
      }else if(fabs(dt0)<=1500){
        prompt[reft0].push_back(coret);
      }else{
        break;
      }
      j++;
    }
  }
  if(!tmp_delay.empty()){
    delay.push_back(tmp_delay);
  }
  tmp_delay.clear();
  for(j;j<core.size();j++) tmp_delay.push_back(core[j]);
  if(!tmp_delay.empty()){
    delay.push_back(tmp_delay);
  }
   
  //std::sort(vvme.begin(),vvme.end());
  //vvme.erase(std::unique(vvme.begin(),vvme.end()),vvme.end());
  //std::sort(vemt.begin(),vemt.end());
  //vemt.erase(std::unique(vemt.begin(),vemt.end()),vemt.end());
  printf("count of emt  = %i\n",emt.size());
  printf("count of core = %i\n",core.size());
  printf("count of delay events = %i\n",delay.size());
  TH1D *hs = new TH1D("hs", "core size correlated to emt", 100,0,100);
  for(auto it=prompt.begin();it!=prompt.end();it++){
    hs->Fill(it->second.size());
  } 
  new TCanvas;
  hs->Draw();
}
