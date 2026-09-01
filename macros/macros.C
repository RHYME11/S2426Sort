{
  double pgacxg[3][2] = {{-18,-13},
                         {-10,-5},
                         {3,8}}; 
  TLine *tl[3][2];
  for(int i=0;i<3;i++){
    for(int j=0;j<2;j++){
      tl[i][j] = new TLine(pgacxg[i][j],0,pgacxg[i][j],2e7);
      tl[i][j]->SetLineWidth(2);
      tl[i][j]->SetLineStyle(9);
      if(i==0) tl[i][j]->SetLineColor(80);
      if(i==1) tl[i][j]->SetLineColor(99);
      if(i==2) tl[i][j]->SetLineColor(1);
    }
  }

  //double pstart[5][2] = {{50  ,500},
  //                       {3000,200},
  //                       {3000,400},
  //                       {50  ,700},
  //                       {50  ,500}};

  //TCutG *cutg[21];
  //double px[5];
  //double py[5];
  //for(int i=0;i<20;i++){
  //  cutg[i] = new TCutG(Form("cutg%i",i),5);
  //  for(int j=0;j<5;j++){
  //    py[j] = pstart[j][1]+i*100;
  //    cutg[i]->SetPoint(j,pstart[j][0], py[j]);
  //    cutg[i]->SetLineColor(i+1);
  //    cutg[i]->SetLineWidth(2);
  //  }
  //}

  TCutG *cutg[21];
  TFile *cutf = TFile::Open("TCutG/physicstree_promptgood/SIIC_banana.root");
  for(int i=0;i<21;i++){
    cutg[i] = (TCutG *)cutf->Get(Form("cutg%i",i));
    cutg[i]->SetLineColor(i+1); 
    cutg[i]->SetLineWidth(2);
  }

  TH2D *pid[4];
  for(int i=0;i<4;i++){
    pid[i] = (TH2D *)_file0->Get(Form("PID/IC%i/IC%i vs Si",i,i));
  }

  TCanvas *c = new TCanvas("c","c");
  c->Divide(2,2);
  for(int i=0;i<4;i++){
    c->cd(i+1);
    gPad->SetLogz();
    pid[i]->Draw("colz");
    for(int j=0;j<21;j++){
      cutg[j]->Draw("same");
    }
    c->Update();
  }

  //cutg[20] = new TCutG("cutg20",5);
  //cutg[20]->SetPoint(0,50,50);
  //cutg[20]->SetPoint(1,3000,50);
  //cutg[20]->SetPoint(2,3000,200);
  //cutg[20]->SetPoint(3,50,500);
  //cutg[20]->SetPoint(4,50,50);

  //TFile *newf = new TFile("TCutG/physicstree_promptgood/SIIC_banana.root","recreate");
  //for(int i=0;i<=20;i++){
  //  cutg[i]->Write();
  //}
  //newf->Close();


}


