{

  //TH1D *tigsize = new TH1D("tigsize","tigress core size in prompt",100,0,100);
  //TH1D *emsize  = new TH1D("emsize" ,"emma size in prompt",100,0,100);

  Tigress *tig = nullptr;
  Emma *em = nullptr;
  prompt->SetBranchAddress("Tigress",&tig);
  prompt->SetBranchAddress("Emma", &em);
  long nentries = prompt->GetEntries();
  long x = 0;
  int count = 0;
  for(x;x<nentries;x++){
    prompt->GetEntry(x);
    if(tig->fCoreHits.size()==0){
      printf("entry = %lu\n",x);
      count++;
    }
  }



}
