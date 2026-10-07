{

  // =========== TCut =========== //
  TCutG *cutg[21];
  TFile *cutf = TFile::Open("TCutG/physicstree_promptgood/SIIC_banana.root");
  for(int i=0;i<21;i++){
    cutg[i] = (TCutG *)cutf->Get(Form("cutg%i",i));
    cutg[i]->SetLineColor(i+1); 
    cutg[i]->SetLineWidth(2);
  }
  // ========== Extract TH3 ============ //
  int cutg_indx = 6; // tcut index
  double pgacx[2] = {-18,-12}; // A/q = 24/9
  //double pgacx[2] = {-9,-3}; //A/q = 29/11
  TH3D *h3 = (TH3D *)_file0->Get(Form("TIG/Coinc/ICs_Si_Gate/IC0/Left/mid ring/gg matrix vs pgacx: Doppler(0.056) within dtns=[-50,150]ns gated ic0 vs si %s mid ring",cutg[cutg_indx]->GetName()));
  h3->GetZaxis()->SetRangeUser(pgacx[0],pgacx[1]);
  TH2D *gg = (TH2D *)h3->Project3D("xy");
  new TBGSubtraction(gg);




}
