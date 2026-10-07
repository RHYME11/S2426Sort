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
  vector<int> pgacx_indx = {1,10,23}; //  pgacx chosen: 0 = (-19,-18], max_index = 28
  int pgacx_len = 5; // how long for each pgacx window
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
  for(int i=0;i<pgacx_indx.size();i++){
    binx.push_back(hxc->FindBin(pgacx_indx[i]-18.5));
    for(int j=0;j<cutg_inds.size();j++){
      hs[i][j][0] = h2[cutg_inds[j]][0]->ProjectionY(Form("hs%i_cutg%i_left",pgacx_indx[i],cutg_inds[j]),binx[i],binx[i]+pgacx_len);
      hs[i][j][1] = h2[cutg_inds[j]][1]->ProjectionY(Form("hs%i_cutg%i_right",pgacx_indx[i],cutg_inds[j]),binx[i],binx[i]+pgacx_len);
    }
  }
  // =========== gamma single subtraction plot ================== //
  int rebin = 2;
  int tcut_indx = 6;
  tcut_indx = 10-tcut_indx;
  int sic = 0; // 0=si.C<150; 1=si.C>150
  TH1D *hsc[3]; // hsc[0]:A/q=24/9; hsc[1,2]:A/q=29/11
  double ymax[2] = {-1,-1};
  for(int i=0;i<2;i++){
    hsc[i] = (TH1D *)hs[i][tcut_indx][sic]->Clone(Form("hsc%i",i));
    hsc[i]->Rebin(rebin);
    hsc[i]->GetXaxis()->SetRangeUser(1350,1380);
    ymax[i] = hsc[i]->GetMaximum();
    hsc[i]->GetXaxis()->SetRangeUser(600,3000);
  }
  hsc[2] = (TH1D *)hsc[1]->Clone("hsc2");
  hsc[2]->GetXaxis()->SetRangeUser(600,3000);
  hsc[0]->SetLineColor(kBlack);
  hsc[1]->SetLineColor(kRed);
  double scalf = ymax[1]/ymax[0];
  printf("%f\n%f\n%f\n",ymax[0],ymax[1],scalf);
  hsc[0]->Scale(scalf);
  hsc[2]->Add(hsc[0],-1);
  TCanvas *c3 = new TCanvas;
  c3->Divide(1,2);
  c3->cd(1);
  hsc[1]->Draw();
  hsc[0]->Draw("same");
  c3->cd(2);
  hsc[2]->Draw("hist");


// ==================================================================================== //

}


