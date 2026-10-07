{

  // =========== TCut =========== //
  TCutG *cutg[12];
  TFile *cutf = TFile::Open("TCutG/physicstree_promptgood/ic0_ic3/IC3IC0_banana.root");
  for(int i=0;i<12;i++){
    cutg[i] = (TCutG *)cutf->Get(Form("cutg%i",i));
    cutg[i]->SetLineColor(i+1); 
    cutg[i]->SetLineWidth(2);
  }

  // ========== PID ============= //
  TH2D *pid[29];
  for(int i=0;i<29;i++){
    pid[i] = (TH2D *)_file0->Get(Form("PID/PGACXGate/%i/ICs/IC0 vs IC3 gated pgacx=[%i,%i)",i-19,i-19,i-18));
    if(pid[i]== nullptr) {printf("%i",i);break;}
    pid[i]->GetXaxis()->SetRangeUser(0,2500);
    pid[i]->GetYaxis()->SetRangeUser(0,2500);
  }


  // ========= TH2: PGACX vs Gamma Ray ========== //
  TH2D *h2[12];
  TH1D *hx[12];
  for(int i=0;i<12;i++){
    h2[i] = (TH2D *)_file0->Get(Form("TIG/IC3_IC0_Gate/PGACX vs Doppler(0.056) gated ic3 vs ic0 cutg%i mid ring",i));
    hx[i] = h2[i]->ProjectionX(Form("hx%i_left",i),1,4e3);
  }

// ========== sum of pgacx with 21 pid tcut gates ========== // 
  TH2D *h2s = (TH2D *)_file0->Get("TIG/PGACX vs Doppler(0.056) mid ring");
  TH1D *hxc = h2s->ProjectionX("hxc",1,4000);
  TCanvas *c0 = new TCanvas("c0","c0");
  hxc->Draw(); 


// ================ Multiple PID and PGACX study ==============//
  vector<int> pgacx_indx = {1,10,23}; //  pgacx chosen: 0 = (-19,-18], max_index = 28
  int pgacx_len = 5; // how long for each pgacx window
  std::vector<int> binx;
  std::vector<int> cutg_inds = {11,10,9,8,7,6,5,4,3,2,1,0}; // tcut chosen
  std::vector<std::vector<TH1D*>> hs(
    pgacx_indx.size(),
    std::vector<TH1D*>(cutg_inds.size(), nullptr)
  );
  TCanvas *c2 = new TCanvas("c2","c2");
  c2->Divide(pgacx_indx.size(),1,0.001,0.001);
  for(int i=0;i<pgacx_indx.size();i++){
    binx.push_back(hxc->FindBin(pgacx_indx[i]-18.5));
    c2->cd(i+1);
    gPad->SetLogz();
    pid[pgacx_indx[i]]->Draw("colz");
    for(int j=0;j<cutg_inds.size();j++){
      c2->cd(i+1);
      cutg[cutg_inds[j]]->Draw("same");
      // == calculate approximate center of TCutG == //
      double xmean = 0;
      double ymean = 0;
      int n = cutg[cutg_inds[j]]->GetN();
      for(int k = 0; k < n; k++){
          double x, y;
          cutg[cutg_inds[j]]->GetPoint(k, x, y);
          xmean += x;
          ymean += y;
      }
      xmean /= n;
      ymean /= n;
      // label cut directly on plot
      TLatex *txt = new TLatex(xmean, ymean, Form("%d", cutg_inds[j]));
      txt->SetTextColor(kBlack);
      txt->SetTextSize(0.04);
      txt->SetTextAlign(22);
      txt->Draw("same");
      // =========================================== //
      c2->Update();
      hs[i][j] = h2[cutg_inds[j]]->ProjectionY(Form("hs%i_cutg%i",pgacx_indx[i],cutg_inds[j]),binx[i],binx[i]+pgacx_len);
    }
    
  }
  // =========== gamma single plot ================== //
  TCanvas *c3 = new TCanvas;
  c3->Divide(1,cutg_inds.size(),0.001,0.001);
  for(int i=0;i<cutg_inds.size();i++){
    int yindx = -1;
    double ymax = -1;
    c3->cd(i+1); // Draw left
    gPad->SetRightMargin(0.04);     
    gPad->SetLeftMargin(0.01);      
    gPad->SetBottomMargin(0.10);    
    gPad->SetTopMargin(0.08);
    for(int j=0;j<pgacx_indx.size();j++){
      hs[j][i]->GetXaxis()->SetRangeUser(600,3000);
      hs[j][i]->SetLineColor(j+1);
      if(ymax<hs[j][i]->GetMaximum()){
        yindx = j;
        ymax = hs[j][i]->GetMaximum();
      }
    }
    hs[yindx][i]->Draw();
    for(int j=0;j<pgacx_indx.size();j++){
      if(j==yindx) continue;
      hs[j][i]->Draw("same");
    }
    TLatex *label = new TLatex();
    label->SetNDC();
    label->SetTextSize(0.15);
    label->SetTextAlign(12); // left-center
    label->DrawLatex(0.99, 0.50, Form("%d", cutg_inds[i]));
    
    // ============================ //
    TPad *pinset2 = new TPad(Form("pinset2_%i",i),"",0.77,0.37,0.95,1.0);                
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
      TLine *tl1 = new TLine(pgacx_indx[j]+pgacx_len-18,0,pgacx_indx[j]+pgacx_len-18,30e6);
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


