{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
  int count0 = 0;
  int count1 = 0;
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->DetType()==8){ 
      count0++;
      if(ftg->Number()==849) count1++;
      else{printf("%s[%i]: address=0x%04x\n",ftg->Name().c_str(),ftg->Number(),ftg->Address());}
    }
  }
  printf("C(DetType=8) = %i\n",count0);
  printf("C(EMT)       = %i\n",count1);
}
