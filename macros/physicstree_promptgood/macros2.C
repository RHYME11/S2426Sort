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
    pid[i]->GetXaxis()->SetRangeUser(0,2500);
    pid[i]->GetYaxis()->SetRangeUser(0,3000);
  }


  // ========= TH2: PGACX vs Gamma Ray ========== //
  TH2D *h2[21][2];
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


// ================ Multiple PID and PGACX study ==============//
  vector<int> pgacx_indx = {4,11,17,24}; //  pgacx chosen: 0 = (-19,-18], max_index = 28
  std::vector<int> binx;
  std::vector<int> cutg_inds = {10,9,8,7,6,5,4,3,2,1,0}; // tcut chosen
  //TH1D *hs[pgacx_indx.size()][cutg_inds.size()][2] = {}; // [pgcx gate][cutg gate][left or right]
  std::vector<std::vector<std::vector<TH1D*>>> hs(
    pgacx_indx.size(),
    std::vector<std::vector<TH1D*>>(
        cutg_inds.size(),
        std::vector<TH1D*>(2, nullptr)
    )
  );
  TCanvas *c2 = new TCanvas("c2","c2");
  c2->Divide(pgacx_indx.size(),2,0.001,0.001);
  TLine *tl3 = new TLine(150,0,150,3000);
  tl3->SetLineColor(kRed);               
  tl3->SetLineStyle(kDashed);            
  for(int i=0;i<pgacx_indx.size();i++){
    binx.push_back(hxc->FindBin(pgacx_indx[i]-18.5));
    c2->cd(i+1);
    gPad->SetLogz();
    pid[pgacx_indx[i]]->Draw("colz");
    tl3->Draw("same");
    c2->cd(i+1+pgacx_indx.size());
    gPad->SetLogz();
    pid[pgacx_indx[i]]->Draw("colz");
    tl3->Draw("same");
    for(int j=0;j<cutg_inds.size();j++){
      if((j%2)==0){
        c2->cd(i+1);
        cutg[cutg_inds[j]]->Draw("same");
        TText *tx = new TText(2200,cutg[cutg_inds[j]]->GetPointY(2)-100,Form("%s",cutg[cutg_inds[j]]->GetName()));
        tx->Draw("same");
      }else{
        c2->cd(i+1+pgacx_indx.size());
        cutg[cutg_inds[j]]->Draw("same");
        TText *tx = new TText(2200,cutg[cutg_inds[j]]->GetPointY(2)-100,Form("%s",cutg[cutg_inds[j]]->GetName()));
        tx->Draw("same");
      }
      c2->Update();
      hs[i][j][0] = h2[cutg_inds[j]][0]->ProjectionY(Form("hs%i_cutg%i_left",pgacx_indx[i],cutg_inds[j]),binx[i],binx[i]);
      hs[i][j][1] = h2[cutg_inds[j]][1]->ProjectionY(Form("hs%i_cutg%i_right",pgacx_indx[i],cutg_inds[j]),binx[i],binx[i]);
    }
    
  }
  // =========== gamma single plot ================== //
  TCanvas *c3 = new TCanvas;
  c3->Divide(2,cutg_inds.size(),0.001,0.001);
  for(int i=0;i<cutg_inds.size();i++){
    int yindx = -1;
    double ymax = -1;
    c3->cd(i*2+1); // Draw left
    gPad->SetRightMargin(0.01);     
    gPad->SetLeftMargin(0.10);      
    gPad->SetBottomMargin(0.10);    
    gPad->SetTopMargin(0.08);
    for(int j=0;j<pgacx_indx.size();j++){
      hs[j][i][0]->GetXaxis()->SetRangeUser(600,3000);
      hs[j][i][0]->SetLineColor(j+1);
      if(ymax<hs[j][i][0]->GetMaximum()){
        yindx = j;
        ymax = hs[j][i][0]->GetMaximum();
      }
    }
    hs[yindx][i][0]->Draw();
    for(int j=0;j<pgacx_indx.size();j++){
      if(j==yindx) continue;
      hs[j][i][0]->Draw("same");
    }
    //TPad *pinset = new TPad(Form("pinset_%i",i),"",0.77,0.37,1.0,1.0);                
    //pinset->Divide(pgacx_indx.size(),1,0.001,0.001);
    //for(int j=0;j<pgacx_indx.size();j++){
    //  pinset->cd(j+1);
    //  gPad->SetLeftMargin(0.12);    
    //  gPad->SetRightMargin(0.12);   
    //  gPad->SetBottomMargin(0.15);  
    //  gPad->SetTopMargin(0.05);     
    //  gPad->Draw();    
    //  gPad->SetLogz();
    //  pid[pgacx_indx[j]]->Draw("colz");
    //  cutg[cutg_inds[i]]->Draw("same");
    //  TLine *tl3 = new TLine(150,0,150,3000);
    //  tl3->SetLineColor(kRed);               
    //  tl3->SetLineStyle(kDashed);            
    //  tl3->Draw("same");
    //  pinset->Update();
    //}
    // ============================ //
    c3->cd(i*2+2);
    gPad->SetRightMargin(0.02);       
    gPad->SetLeftMargin(0.06);      
    gPad->SetBottomMargin(0.10);      
    gPad->SetTopMargin(0.08);        
    yindx = -1;
    ymax = -1; 
    for(int j=0;j<pgacx_indx.size();j++){
      hs[j][i][1]->GetXaxis()->SetRangeUser(600,3000);
      hs[j][i][1]->SetLineColor(j+1);
      if(ymax<hs[j][i][1]->GetMaximum()){
        yindx = j;
        ymax = hs[j][i][1]->GetMaximum();
      }
    }
    hs[yindx][i][1]->Draw();
    for(int j=0;j<pgacx_indx.size();j++){
      if(j==yindx) continue;
      hs[j][i][1]->Draw("same");
    }
    TPad *pinset2 = new TPad(Form("pinset2_%i",i),"",0.77,0.37,1.0,1.0);                
    pinset2->SetLeftMargin(0.12);      
    pinset2->SetRightMargin(0.12);     
    pinset2->SetBottomMargin(0.15);    
    pinset2->SetTopMargin(0.05);       
    pinset2->Draw();                   
    pinset2->cd();
    hxc->GetXaxis()->SetRangeUser(-25,20);
    hxc->Draw();
    for(int j=0;j<pgacx_indx.size();j++){
      TLine *tl0 = new TLine(pgacx_indx[j]-19,0,pgacx_indx[j]-19,30e6);  
      TLine *tl1 = new TLine(pgacx_indx[j]-18,0,pgacx_indx[j]-18,30e6);
      tl0->SetLineColor(j+1);            
      tl1->SetLineColor(j+1);            
      tl0->SetLineStyle(kDashed);         
      tl0->Draw("same");                  
      tl1->Draw("same");                  
    }
    pinset2->Update();
    c3->Update();
  }  



// ==================================================================================== //

}


