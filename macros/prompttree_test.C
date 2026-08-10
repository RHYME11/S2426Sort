{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  DetectorEvent *eve = nullptr;
  PromptTree->SetBranchAddress("event", &eve);
  long nentries = PromptTree->GetEntries();
  long x = 0;
  int count = 0;
  for(x;x<nentries;x++){
    PromptTree->GetEntry(x);
    if(eve->tigress.fCoreHits.size()==0 && eve->emma.ADC().size()==0 && eve->emma.TDC().size()==0){
      printf("entry = %lu\n",x);
      count++;
    }
  }
  printf("count=%i\n",count)


}
