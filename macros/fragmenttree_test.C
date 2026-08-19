{

  //Channel::Read("cal/CalibrationFile_Nov182025.cal");
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);                              
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;

  TH1D *hpileup = new TH1D("hpileup", "Core: # of pile up", 100,0,100);
  TH2D *h[64];
  for(int i=0;i<64;i++){
    h[i] = new TH2D(Form("h%i",i),Form("KValue vs Energy at Array%i",i),500,0,500,4e3,0,4e3);
  } 
  int count0, count1, count2, count3; 
  for(x;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    long thisTime = ftg->TimestampNs();
    if(ftg->DetType()!=0) continue;
    count0++;
    int arrynumber = ftg->Number()/15 + 16;
    h[arrynumber]->Fill(ftg->KValue(), ftg->Energy());
    if(ftg->KValue()==379) count1++;
    if(ftg->Pileup() == 0) count2++; 
    if(ftg->Pileup() == 1) count3++;
    hpileup->Fill(ftg->Pileup()); 
  }

  TCanvas *c = new TCanvas;
  c->Divide(8,6);
  for(int i=0;i<64;i++){
    if(i<16) continue;
    c->cd(i-15);
    h[i]->Draw();
    c->Update();
  }
  new TCanvas; hpileup->Draw();

  printf("C(core) = %i\n",count0);
  printf("C(core: KValue=379) = %i\n",count1);
  printf("C(core: Pileup=0)   = %i\n",count2);
  printf("C(core: Pileup=1)   = %i\n",count3);
  
}
