{

  //Channel::Read("cal/CalibrationFile_Nov182025.cal");
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;

  TH1D *h[64];
  for(int i=0;i<64;i++){
    h[i] = new TH1D(Form("h%i",i),Form("Core: dt = fabs(Time()-TimestampNs()) Array%i",i),4e6,0,4e6);
  } 
  int count0, count1, count2, count3; 
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->DetType()!=0) continue;
    int arrynumber = ftg->Number()/15 + 16;
    double dt = fabs(ftg->Time()-ftg->TimestampNs());
    h[arrynumber]->Fill(dt);
  }

  TCanvas *c = new TCanvas;
  c->Divide(8,6);
  for(int i=0;i<64;i++){
    if(i<16) continue;
    c->cd(i-15);
    h[i]->Draw();
    c->Update();
  }
  
}
