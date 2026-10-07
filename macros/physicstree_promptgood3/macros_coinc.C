{

  // =========== TCut =========== //
  TCutG *cutg[12];
  TFile *cutf = TFile::Open("TCutG/physicstree_promptgood/ic0_ic3/IC3IC0_banana.root");
  for(int i=0;i<12;i++){
    cutg[i] = (TCutG *)cutf->Get(Form("cutg%i",i));
    cutg[i]->SetLineColor(i+1); 
    cutg[i]->SetLineWidth(2);
  }
  // ========== Extract TH3 ============ //
  int cutg_indx = 7; // tcut index
  double pgacx[2] = {-18,-12};
  TH3D *h3 = (TH3D *)_file0->Get(Form("TIG/Coinc/IC3_IC0_Gate/mid ring/gg matrix vs pgacx: Doppler(0.056) within dtns=[-50,150]ns gated %s mid ring",cutg[cutg_indx]->GetName()));
  h3->GetZaxis()->SetRangeUser(pgacx[0],pgacx[1]);
  TH2D *gg = (TH2D *)h3->Project3D("xy");
  new TBGSubtraction(gg);




}
