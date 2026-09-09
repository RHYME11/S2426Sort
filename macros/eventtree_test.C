{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  DetectorEvent *eve = nullptr;
  EventTree->SetBranchAddress("DetectorEvent", &eve);
  long nentries = EventTree->GetEntries();
  long x = 0;
  int count0, count1, count2, count3,count4, count5,count6, count7,count8, count9,count10;
  for(x;x<nentries;x++){
    EventTree->GetEntry(x);
    bool emv = false;
    bool emt = false;
    for(int m=0;m<eve->Fragments().size();m++){
      int dettype = eve->Fragments()[m].DetType();
      if(dettype == 8) emt = true;
      if(dettype ==13 || dettype == 14) emv = true;
    }
    if(emv){
      count0++;
      if(emt) count1++;
      else count2++;
    }
    if(emt && !emv) count3++;
  }

  printf("total events           = %lu\n",EventTree->GetEntries());
  printf("events have emv        = %d\n", count0);
  printf("events have emv && emt = %d\n", count1);
  printf("events have emv no emt = %d\n", count2);
  printf("events have emt no emv = %d\n", count3);

}
