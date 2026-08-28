{
  //TFile *infile = TFile::Open("histOutput/physicstree_promptgood/master.root");
  TH2D *hic2si[4];
  hic2si[0] = (TH2D *)_file0->Get("PID/IC2/IC2 vs Si");
  hic2si[1] = (TH2D *)_file0->Get("PID/IC2/PGACX=[-18,-13]: IC2 vs Si");
  hic2si[2] = (TH2D *)_file0->Get("PID/IC2/PGACX=[-10,-5]: IC2 vs Si");
  hic2si[3] = (TH2D *)_file0->Get("PID/IC2/PGACX=[3,8]: IC2 vs Si");

  GCanvas *c0 = new GCanvas("c0","c0");
  c0->Divide(3,2,0,0);
  c0->cd(1);hic2si[0]->Draw("colz");hic2si[1]->Draw("same scat");
  c0->cd(2);hic2si[0]->Draw("colz");hic2si[2]->Draw("same scat");
  c0->cd(3);hic2si[0]->Draw("colz");hic2si[3]->Draw("same scat");
  c0->cd(4);hic2si[1]->Draw("colz");
  c0->cd(5);hic2si[2]->Draw("colz");
  c0->cd(6);hic2si[3]->Draw("colz");

  // ============================ //
  TH2D *hpgac = (TH2D *)_file0->Get("TIG/PGACX vs Doppler(0.056)");
  TLine *tl[3][2];
  tl[0][0] = new TLine(-18,0,-18,22e6);
  tl[0][1] = new TLine(-13,0,-13,22e6);
  tl[1][0] = new TLine(-10,0,-10,22e6);
  tl[1][1] = new TLine(-5 ,0,-5 ,22e6);
  tl[2][0] = new TLine(3  ,0,3  ,22e6);
  tl[2][1] = new TLine(8  ,0,8  ,22e6);

  tl[0][0]->SetLineStyle(9);  tl[0][0]->SetLineColor(kGreen);
  tl[0][1]->SetLineStyle(9);  tl[0][1]->SetLineColor(kGreen);
  tl[1][0]->SetLineStyle(9);  tl[1][0]->SetLineColor(kRed);
  tl[1][1]->SetLineStyle(9);  tl[1][1]->SetLineColor(kRed);
  tl[2][0]->SetLineStyle(9);  tl[2][0]->SetLineColor(kOrange);
  tl[2][1]->SetLineStyle(9);  tl[2][1]->SetLineColor(kOrange);

  GCanvas *c1 = new GCanvas("c1","c1");
  hpgac->ProjectionX()->Draw();
  tl[0][0]->Draw("same");
  tl[0][1]->Draw("same");
  tl[1][0]->Draw("same");
  tl[1][1]->Draw("same");
  tl[2][0]->Draw("same");
  tl[2][1]->Draw("same");


}
