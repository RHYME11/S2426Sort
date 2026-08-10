{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;
  int count=0;
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->DetType()==8) count++;
  }
  printf("count = %i\n",count);
}
