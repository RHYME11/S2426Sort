{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  DetectorEvent *eve = nullptr;
  PromptTree->SetBranchAddress("event", &eve);
  long nentries = PromptTree->GetEntries();
  long x = 1642110;
  int count = 0;
  for(x;x<nentries;x++){
    PromptTree->GetEntry(x);
    if(eve->emma.ADC().size()==0 && eve->emma.TDC().size()==0){
      printf("entry = %lu, EMT=%lu\n",x,eve->timestampNs);
      break;  
      //count++;
    }
  }
  //printf("count=%i\n",count)


}
