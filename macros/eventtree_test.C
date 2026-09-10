{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  DetectorEvent *eve = nullptr;
  EventTree->SetBranchAddress("DetectorEvent", &eve);
  long nentries = EventTree->GetEntries();
  long x = 0;
  int count0, count1, count2, count3,count4, count5,count6, count7,count8, count9,count10;
  std::vector<int> vemt;
  for(x;x<nentries;x++){
    EventTree->GetEntry(x);
    bool emv = false;
    bool emt = false;
    int count = 0;
    for(int m=0;m<eve->Fragments().size();m++){
      int dettype = eve->Fragments()[m].DetType();
      if(dettype == 8) {emt = true; count++;}
      if(dettype ==13 || dettype == 14) emv = true;
    }
    if(emv){
      count0++;
      if(emt) count1++;
      else count2++;
    }
    if(emt && !emv) {count3++; vemt.push_back(count);printf("entry = %lu\n",x);}
  }

  printf("total events           = %lu\n",EventTree->GetEntries());
  printf("events have vme        = %d\n", count0);
  printf("events have vme && emt = %d\n", count1);
  printf("events have vme no emt = %d\n", count2);
  printf("events have emt no vme = %d\n", count3);

}
