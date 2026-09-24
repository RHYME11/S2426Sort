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

  // =========== TCut =========== //
  TCutG *cutg[21];
  TFile *cutf = TFile::Open("TCutG/physicstree_promptgood/SIIC_banana.root");
  for(int i=0;i<21;i++){
    cutg[i] = (TCutG *)cutf->Get(Form("cutg%i",i));
    cutg[i]->SetLineColor(i+1); 
    cutg[i]->SetLineWidth(2);
  }

  // ========== PID ============= //
  TH2D *pid[29];
  for(int i=0;i<29;i++){
    pid[i] = (TH2D *)_file0->Get(Form("PID/PGACXGate/%i/ICs/IC0 vs Si gated pgacx=[%i,%i)",i-19,i-19,i-18));
    if(pid[i]== nullptr) {printf("%i",i);break;}
    pid[i]->GetXaxis()->SetRangeUser(0,3500);
    pid[i]->GetYaxis()->SetRangeUser(0,3000);
  }


  // ========= TH2: PGACX vs Gamma Ray ========== //
  TH2D *h2[21][2];
  TH1D *hs[21][2];
  TH1D *hx[21][2];
  for(int i=0;i<21;i++){
    h2[i][0] = (TH2D *)_file0->Get(Form("TIG/ICs_Si_Gate/IC0/Left/PGACX vs Doppler(0.056) gated ic0 vs si cutg%i mid ring",i));
    h2[i][1] = (TH2D *)_file0->Get(Form("TIG/ICs_Si_Gate/IC0/Right/PGACX vs Doppler(0.056) gated ic0 vs si cutg%i mid ring",i));
    hx[i][0] = h2[i][0]->ProjectionX(Form("hx%i_left",i),1,4e3);
    hx[i][1] = h2[i][1]->ProjectionX(Form("hx%i_right",i),1,4e3);
  }

// ========== sum of pgacx with 21 pid tcut gates ========== // 
  TH1D *hxc = (TH1D *)hx[0][0]->Clone("hxc");
  hxc->Add(hx[0][1]);
  for(int i=1;i<21;i++){
    hxc->Add(hx[i][0]);
    hxc->Add(hx[i][1]);
  }
  TCanvas *c0 = new TCanvas("c0","c0");
  hxc->Draw(); 

// ========== Draw PGACX gated IC0 vs Si ============= //
  /*TCanvas *c1[3];
  for(int i=0;i<3;i++){
    c1[i] = new TCanvas(Form("c1_%i",i),Form("c1_%i",i));
    c1[i]->Divide(2,5,0.001,0.001);
    for(int j=0;j<10;j++){
      if((10*i+j)>28) continue;
      c1[i]->cd(j+1);
      pid[i*10+j]->Draw("colz");
      gPad->SetRightMargin(0.12);     
      gPad->SetLeftMargin(0.10);      
      gPad->SetBottomMargin(0.10);    
      gPad->SetTopMargin(0.08);       
      gPad->SetLogz();          
      gPad->Update();
      TPad *pinset = new TPad(Form("pinset_%i",10*i+j),"",0.62,0.67,1.0,1.0);
      pinset->SetLeftMargin(0.12);
      pinset->SetRightMargin(0.12);
      pinset->SetBottomMargin(0.15);
      pinset->SetTopMargin(0.05);
      pinset->Draw();    
      pinset->cd();
      hxc->Draw();
      int xp = 10*i+j-20;
      TLine *tl0 = new TLine(xp,0,xp,30e6);
      TLine *tl1 = new TLine(xp+1,0,xp+1,30e6);
      tl0->SetLineColor(kRed);
      tl1->SetLineColor(kRed);
      tl0->SetLineStyle(kDashed);
      tl0->Draw("same");
      tl1->Draw("same");
      pinset->Update();              
    } // loop j over
    c1[i]->Update();
  } // loop i over
  */
// ================ Individual PID and PGACX study ==============//
  int pgacx_indx = 1; //  pgacx chosen: 0 = (-19,-18]
  pid[pgacx_indx]->GetYaxis()->SetRangeUser(0,2500);
  TCanvas *c2 = new TCanvas("c2","c2");
  c2->Divide(3,1,0.001,0.001);
  c2->cd(1);
  pid[pgacx_indx]->Draw("colz");
  gPad->SetLogz();
  c2->cd(2);
  pid[pgacx_indx]->Draw("colz");
  gPad->SetLogz();
  c2->cd(3);
  pid[pgacx_indx]->Draw("colz");
  gPad->SetLogz();
  for(int i=0;i<20;i++){
    if(i%2==0){
      c2->cd(2);
      cutg[i]->Draw("same");
    }else{
      c2->cd(3);
      cutg[i]->Draw("same");
    }
    c2->Update();
  }
  int binx = hxc->FindBin(pgacx_indx-18.5);
  std::vector<int> cutg_inds = {8,7,6,5,2,0}; // tcut chosen
  TCanvas *c3 = new TCanvas;
  c3->Divide(2,cutg_inds.size(),0.001,0.001);
  for(int i=0;i<cutg_inds.size();i++){
    TH1D *htempl = h2[cutg_inds[i]][0]->ProjectionY(Form("hs_%i_left",cutg_inds[i]),binx,binx);
    c3->cd(i*2+1);
    htempl->Draw();
    gPad->SetRightMargin(0.12);     
    gPad->SetLeftMargin(0.10);      
    gPad->SetBottomMargin(0.10);    
    gPad->SetTopMargin(0.08);
    TPad *pinset = new TPad(Form("pinset_%i",i),"",0.77,0.37,1.0,1.0);                
    pinset->SetLeftMargin(0.12);    
    pinset->SetRightMargin(0.12);   
    pinset->SetBottomMargin(0.15);  
    pinset->SetTopMargin(0.05);     
    pinset->Draw();    
    pinset->SetLogz();
    pinset->cd();
    pid[pgacx_indx]->Draw("colz");
    cutg[cutg_inds[i]]->Draw("same");
    TLine *tl3 = new TLine(150,0,150,3000);
    tl3->SetLineColor(kRed);               
    tl3->SetLineStyle(kDashed);            
    tl3->Draw("same");
    pinset->Update();
    // ============================ //
    TH1D *htempr = h2[cutg_inds[i]][1]->ProjectionY(Form("hs_%i_right",cutg_inds[i]),binx,binx);
    c3->cd(i*2+2);
    htempr->Draw();
    gPad->SetRightMargin(0.12);       
    gPad->SetLeftMargin(0.10);      
    gPad->SetBottomMargin(0.10);      
    gPad->SetTopMargin(0.08);         
    TPad *pinset2 = new TPad(Form("pinset2_%i",i),"",0.77,0.37,1.0,1.0);                
    pinset2->SetLeftMargin(0.12);      
    pinset2->SetRightMargin(0.12);     
    pinset2->SetBottomMargin(0.15);    
    pinset2->SetTopMargin(0.05);       
    pinset2->Draw();                   
    pinset2->cd();
    hxc->GetXaxis()->SetRangeUser(-25,20);
    hxc->Draw();
    TLine *tl0 = new TLine(pgacx_indx-19,0,pgacx_indx-19,30e6);  
    TLine *tl1 = new TLine(pgacx_indx-18,0,pgacx_indx-18,30e6);
    tl0->SetLineColor(kRed);            
    tl1->SetLineColor(kRed);            
    tl0->SetLineStyle(kDashed);         
    tl0->Draw("same");                  
    tl1->Draw("same");                  
    pinset2->Update();
    c3->Update();
  }  



// ==================================================================================== //

}


