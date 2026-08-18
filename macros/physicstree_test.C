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
  TH2D *sum0 = new TH2D("sum0","Summary",64,0,64,1e4,0,1e4);
  TH2D *sum1 = new TH2D("sum1",Form("Summary: Doppler cor (b=%.3f)",beta),64,0,64,1e4,0,1e4);
  TH2D *sum2 = new TH2D("sum2",Form("Summary: Doppler cor (b=%.3f) + BGO veto",beta),64,0,64,1e4,0,1e4);
  TH2D *dt0 = new TH2D("dt0","dt(Time) between two gam vs Doppler(b=0.056)",6e2,-3e3,3e3,2e3,0,8e3);
  TH2D *dt1 = new TH2D("dt1","dt(TimestampNs) between two gam vs Doppler(b=0.056)",6e2,-3e3,3e3, 2e3,0,8e3);
  TH1D *dt2 = new TH1D("dt2","dt(TimestampNs) between two gam if fabs(dTime)>3us",6e2,-3e3,3e3);
  TH1D *dt3 = new TH1D("dt3","dt = fabs(Time() - TimestampNs())",4e6,0,4e6);
  int count0, count1, count2, count3,count4, count5,count6, count7,count8, count9,count10;
  for(x=0;x<nentries;x++){
    tree->GetEntry(x);
    if(emma->Left().size()>0 && emma->Right().size()>0){
      for(int i=0;i<tig->Hits().size();i++){
        double ei = tig->Hits()[i].Doppler(beta);
        if(ei<15) continue;
        double ti = tig->Hits()[i].Time();
        bool bgoi = tig->Hits()[i].BGOFire();
        int arryi = tig->Hits()[i].ArrayNumber();
        sum0->Fill(arryi,tig->Hits()[i].Energy());
        sum1->Fill(arryi,ei);
        if(!bgoi) sum2->Fill(arryi,ei);
        dt3->Fill(fabs(ti - tig->Hits()[i].TimestampNs()));
        /*for(int j=i+1;j<tig->Hits().size();j++){
          double ej = tig->Hits()[j].Doppler(beta);
          if(ej<15) continue;
          double tj = tig->Hits()[j].Time();
          bool bgoj = tig->Hits()[j].BGOFire();
          double dt = 0;
          double dtns = 0;
          double e = -1;
          if(ei<ej) {dt = ti - tj; e = ej; dtns = tig->Hits()[i].TimestampNs() - tig->Hits()[j].TimestampNs();}
          else      {dt = tj - ti; e = ei; dtns = tig->Hits()[j].TimestampNs() - tig->Hits()[i].TimestampNs();}
          dt0->Fill(dt,e);
          dt1->Fill(dtns,e);
          if(fabs(dt)>=3000) dt2->Fill(dtns);
          //if((!bgoi) && (!bgoj)) dt1->Fill(dt);
        } // j loop over 
        */ 
      } // i loop over
    } // if left && right over
  } // tree loop over

  //new TCanvas; sum0->Draw();
  //new TCanvas; sum1->Draw();
  //new TCanvas; sum2->Draw();
  new TCanvas; dt0 ->Draw();
  new TCanvas; dt1 ->Draw();
  //new TCanvas; dt2 ->Draw();
  //new TCanvas; dt3 ->Draw();

}
