{                                                         
                                                          
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Tigress *tig = nullptr;
  Emma *emma = nullptr;
  TTree *tree = (TTree *)_file0->Get("PromptGoodTree");
  tree->SetBranchAddress("Tigress", &tig); 
  tree->SetBranchAddress("Emma",    &emma); 
  long nentries = tree->GetEntries();                
  long x = 0;                                             
  
  double beta = 0.056;
  TH1D *hs = new TH1D("hs","single gamma(phytree)",8e3,0,8e3);
  int count0, count1, count2, count3,count4, count5,count6, count7,count8, count9,count10;
  for(x=0;x<nentries;x++){
    tree->GetEntry(x);
    if(emma->Left().size()>0 && emma->Right().size()>0){
      for(int i=0;i<tig->Hits().size();i++){
        if(tig->Hits()[i].KValue()!=379) continue;
        double e = tig->Hits()[i].Energy();
        if(e>40)hs->Fill(e);
      } // i loop over
    } // if left && right over
  } // tree loop over

  new TCanvas; hs ->Draw();

}
