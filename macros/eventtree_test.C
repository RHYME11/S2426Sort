{

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  DetectorEvent *eve = nullptr;
  EventTree->SetBranchAddress("DetectorEvent", &eve);
  long nentries = EventTree->GetEntries();
  long x = 0;
  int count0, count1, count2, count3,count4, count5,count6, count7,count8, count9,count10;
  TH1D *hbgo[12][4][5];       
  for(int i=0;i<12;i++){      
    for(int j=0;j<4;j++){     
      for(int k=0;k<5;k++){   
        hbgo[i][j][k] = new TH1D(Form("hbgo_%i_%i_%i",i,j,k),Form("Det%i, Xtal%i, BGO%i: Charge",i+5,j,k),16000,0,16000);
      }              
    }                
  }
  for(x;x<nentries;x++){
    EventTree->GetEntry(x);
    for(int m=0;m<eve->Fragments().size();m++){
      int number = eve->Fragments()[m].Number();
      string name = eve->Fragments()[m].Name();
      if(number>=720 || (name.substr(0,3)!="TIS")) continue;
      int i = stoi(name.substr(3,2))-5;
      int j;
      if (name[5] == 'B') j = 0;
      else if (name[5] == 'G') j = 1;
      else if (name[5] == 'R') j = 2;
      else if (name[5] == 'W') j = 3;
      else continue;
      int k = stoi(name.substr(7,2))-1;
      hbgo[i][j][k]->Fill(eve->Fragments()[m].Charge());
    }
  }
  TCanvas *c[12]; 
  TLine *tl = new TLine(20,0,20,1e6);
  tl->SetLineColor(kRed); 
  for(int i=0;i<12;i++){
    c[i] = new TCanvas(Form("c%i",i),Form("c%i",i));
    c[i]->Divide(4,5);
    for(int j=0;j<4;j++){
      for(int k=0;k<5;k++){
        c[i]->cd(5*j+k+1);
        gPad->SetLogy();
        if(hbgo[i][j][k] != nullptr){
          hbgo[i][j][k]->Draw();
          tl->Draw("same");
        }
      }
    }
    c[i]->Update();
  }
  TFile *fout = new TFile("output.root", "RECREATE");
  for(int i=0; i<12; i++){
    c[i]->Write();
  }
  fout->Close();
}
