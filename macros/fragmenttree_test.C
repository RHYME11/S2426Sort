{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
 
  int count0, count1; 
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    long thisTime = ftg->TimestampNs();
    if(ftg->DetType()!=0) continue;
    count0++;
    if(ftg->Pileup()==0) count1++;
  }
  printf("C(core) = %i\n",count0);
  printf("C(core: no pileup) = %i\n",count1);
  
}
