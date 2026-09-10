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
  for(double t:emt) prompt[t] = {};
  std::vector<std::vector<double>> delay; 
  int j=0;
  for(int i=0;i<emt.size();i++){
    double reft = emt[i];
    std::vector<double> tmp_delay;
    while(j<core.size() && core[j]-reft<-1500){
      tmp_delay.push_back(core[j++]);
    }
    if(!tmp_delay.empty()) delay.push_back(tmp_delay);
    while(j<core.size() && core[j]-reft<=1500){
      prompt[reft].push_back(core[j++]);
    }
  }
  if(j<core.size()) delay.emplace_back(core.begin()+j,core.end());
  
  printf("count of emt  = %i\n",emt.size());
  printf("count of core = %i\n",core.size());
  printf("count of delay events = %i\n",delay.size());
    

}
  
