{

  Channel::Read("cal/CalibrationFile_Nov182025.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
 
  int count0, count1, count2; 
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    long thisTime = ftg->TimestampNs();
    if(ftg->DetType()!=0) continue;
    count0++;
    if(ftg->Pileup()==0) count1++;
    if(ftg->Pileup()==1) count2++;
  }
  printf("C(core) = %i\n",count0);
  printf("C(core: pileup=0) = %i\n",count1);
  printf("C(core: pileup=1) = %i\n",count2);
  
}
