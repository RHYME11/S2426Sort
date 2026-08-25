{

  TFile *infile = TFile::Open("histOutput/physicstree_promptgood/master.root");
  TFile *cutf = TFile::Open("TCutG/physicstree_promptgood/SiIC0_gamgated_banana.root");
  TH2D *h[6];
  h[0] = (TH2D *)infile->Get("PID/Gate/BGOveto/Si vs IC0: Doppler(0.056)=[1200,1250]keV && pGACX=[-5,-10]");
  h[1] = (TH2D *)infile->Get("PID/Gate/BGOveto/Si vs IC0: Doppler(0.056)=[1250,1300]keV && pGACX=[-5,-10]");
  h[2] = (TH2D *)infile->Get("PID/Gate/BGOveto/Si vs IC0: Doppler(0.056)=[1330,1380]keV && pGACX=[-5,-10]");
  h[3] = (TH2D *)infile->Get("PID/Gate/BGOveto/Si vs IC0: Doppler(0.056)=[1200,1250]keV && pGACX=[-12,-17]");
  h[4] = (TH2D *)infile->Get("PID/Gate/BGOveto/Si vs IC0: Doppler(0.056)=[1250,1300]keV && pGACX=[-12,-17]");
  h[5] = (TH2D *)infile->Get("PID/Gate/BGOveto/Si vs IC0: Doppler(0.056)=[1330,1380]keV && pGACX=[-12,-17]");
  TCutG *cutg[2];
  cutg[0] = (TCutG *)cutf->Get("siic0_top");
  cutg[1] = (TCutG *)cutf->Get("siic0_bot");
  /*GCanvas *c0 = new GCanvas("c0","c0");
  c0->Divide(3,2);
  GCanvas *c1 = new GCanvas("c1","c1");
  c1->Divide(3,2);
  GCanvas *c2 = new GCanvas("c2","c2");
  c2->Divide(3,2);
  TH1D *ht[6];
  TH1D *hb[6];
  for(int i=0;i<6;i++){
    c0->cd(1+i);
    h[i]->Draw(); cutg[0]->Draw("same"); cutg[1]->Draw("same");
    ht[i] = h[i]->ProjectionY(Form("ht_%i",i),1,4e3,"[siic0_top]");
    hb[i] = h[i]->ProjectionY(Form("hb_%i",i),1,4e3,"[siic0_bot]");
    ht[i]->Rebin(4);
    hb[i]->Rebin(4);
    c1->cd(i+1);
    ht[i]->Draw();
    c2->cd(i+1);
    hb[i]->Draw();
    c0->Update();
    c1->Update();
    c2->Update();
  }
  */
  TH2D *hsiic0_right = (TH2D *)infile->Get("PID/Gate/PGACX/PGACX=[-10,-5]: Si vs IC0");
  TH2D *hsiic0_left  = (TH2D *)infile->Get("PID/Gate/PGACX/PGACX=[-12,-17]: Si vs IC0");
  GCanvas *c3 = new GCanvas("c3","c3");
  c3->Divide(2,1);
  c3->cd(1); hsiic0_left ->Draw(); cutg[0]->Draw("same"); cutg[1]->Draw("same");
  c3->cd(2); hsiic0_right->Draw(); cutg[0]->Draw("same"); cutg[1]->Draw("same");


}
