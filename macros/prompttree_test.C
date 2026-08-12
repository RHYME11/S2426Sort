{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  DetectorEvent *eve = nullptr;
  PromptTree->SetBranchAddress("event", &eve);
  long nentries = PromptTree->GetEntries();
  long x = 0;
  int count0, count1, count2, count3,count4, count5,count6, count7,count8, count9,count10;
  for(x;x<nentries;x++){
    PromptTree->GetEntry(x);
    if(eve->emma.ADC().size()==0 && eve->emma.TDC().size()==0){
      //printf("entry = %lu, EMT=%lu\n",x,eve->timestampNs);
      //break;  
      count0++;
    }
    if(eve->tigress.fCoreHits.size()>0){
      count5++;
    }
    if(eve->emma.Anodes().size()>0 && eve->emma.Left().size()>0 && eve->emma.Right().size()>0 && eve->emma.Bot().size()>0 && eve->emma.Top().size()>0){
      count6++;
    }
    if(eve->emma.Anodes().size()>0 && eve->emma.Left().size()>0 && eve->emma.Right().size()>0 && eve->emma.Bot().size()>0 && eve->emma.Top().size()>0 && eve->tigress.fCoreHits.size()>0){
      count7++;
    }
  }
  printf("C(no ADC && no TDC)=%i\n",count0);
  printf("C(has tig)         =%i\n",count5);
  printf("C(has pgac)        =%i\n",count6);
  printf("C(has tig && pgac) =%i\n",count7);

}
