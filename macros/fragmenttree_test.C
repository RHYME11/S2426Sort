{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
  map<long,vector<Fragment>> emma_map;
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

}
