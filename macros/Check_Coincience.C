#include<iostream>
#include<algorithm>

int rebin=1;
void coinc(TH2D *mat, double lower, double upper){

  //int rebin = 8;

  mat->GetXaxis()->UnZoom();
  mat->GetYaxis()->UnZoom();

  int lower_bin = mat->GetXaxis()->FindBin(lower);
  int upper_bin = mat->GetXaxis()->FindBin(upper)-1;
  
  TH1D *tot = mat->ProjectionX(Form("%s_tot",mat->GetName()));
  tot->SetTitle(Form("%s total projection", mat->GetTitle()));
  tot->GetXaxis()->SetRangeUser(lower-4, upper+4);

  TH1D *px = mat->ProjectionX(Form("%s_px",mat->GetName()),lower_bin,upper_bin);
  px->SetTitle(Form("%s projected [%.1f, %.1f]keV in bin[%i,%i]", mat->GetTitle(), lower, upper, lower_bin, upper_bin));  
  px->Rebin(rebin);

  double ymax = tot->GetMaximum();
  TLine *l1 = new TLine(lower,0 ,lower,ymax);
  TLine *l2 = new TLine(upper,0 ,upper,ymax);
  l1->SetLineColor(kRed);
  l2->SetLineColor(kRed);

  TCanvas *c = new TCanvas();
  c->Divide(1,2);
  c->cd(1);tot->Draw("hist");l1->Draw("same");l2->Draw("same");
  c->cd(2);px->Draw("hist");

}


void coinc_bg1(TH2D *mat, double lower, double upper, double bgl=-1, double bgr=-1){
  
  gStyle->SetOptStat(0);
  
  //int rebin = 8;

  mat->GetXaxis()->UnZoom();
  mat->GetYaxis()->UnZoom();

  if(bgl<0 || bgr<0){
    bgl = upper;
    bgr = bgl + (upper - lower);
  }
  
  if(bgl>bgr){
    double temp = bgr;
    bgr = bgl;
    bgl = temp;
  }
  
  int lower_bin = mat->GetXaxis()->FindBin(lower);
  int upper_bin = mat->GetXaxis()->FindBin(upper)-1;
  int bgl_bin   = mat->GetXaxis()->FindBin(bgl);
  int bgr_bin   = mat->GetXaxis()->FindBin(bgr)-1;
  
  TH1D *px = mat->ProjectionX(Form("%s_px",mat->GetName()),lower_bin,upper_bin);
  px->SetTitle(Form("%s projected [%.1f, %.1f]keV in bin[%i,%i]", mat->GetTitle(), lower, upper, lower_bin, upper_bin));  
  px->Rebin(rebin);

  TH1D *bg = mat->ProjectionX(Form("%s_bg",mat->GetName()),bgl_bin,bgr_bin);
  bg->SetTitle(Form("%s BG [%.1f, %.1f]keV in bin[%i,%i]", mat->GetTitle(), bgl, bgr, bgl_bin, bgr_bin));  
  bg->Rebin(rebin);

  TH1D *cpx = (TH1D *)px->Clone(Form("%s_cl",px->GetName()));
  cpx->Add(bg,-1);
  cpx->SetTitle(Form("%s projected[%.1f, %.1f] subbg [%.1f, %.1f] (bin:[%i,%i] - [%i,%i])", mat->GetTitle(),lower,upper,bgl,bgr, lower_bin,upper_bin,bgl_bin, bgr_bin));
  cpx->SetMinimum(0);

  TH1D *tot = mat->ProjectionX(Form("%s_tot",mat->GetName()));
  tot->SetTitle(Form("%s total projection", mat->GetTitle()));
  double mn = std::min({lower,upper,bgl,bgr});
  double mx = upper;
  if(bgr>upper) mx = bgr;
  tot->GetXaxis()->SetRangeUser(mn-4, mx+4);

  double ymax = tot->GetMaximum();
  TLine *l1   = new TLine(lower,0 ,lower,ymax);
  TLine *l2   = new TLine(upper,0 ,upper,ymax);
  TLine *lbgl = new TLine(bgl  ,0 ,bgl  ,ymax);
  TLine *lbgr = new TLine(bgr  ,0 ,bgr  ,ymax);
  l1  ->SetLineColor(kRed);
  l2  ->SetLineColor(kRed);
  lbgl->SetLineColor(kGreen);
  lbgr->SetLineColor(kGreen);

  //px ->GetXaxis()->SetRangeUser(650, 2000);
  //cpx->GetXaxis()->SetRangeUser(650, 2000);

  TCanvas *c = new TCanvas();
  c->Divide(1,3);
  c->cd(1);tot->Draw("hist");l1->Draw("same");l2->Draw("same");lbgl->Draw("same");lbgr->Draw("same");
  c->cd(2);px ->Draw("hist");bg->Draw("same hist");
  c->cd(3);cpx->Draw("hist");

  px->SetLineWidth(2);
  bg->SetLineColor(kRed);

}


void coinc_bg2(TH2D *mat, double lower, double upper, double bgl1=-1, double bgr1=-1, double bgl2=-1, double bgr2=-1){
  
  gStyle->SetOptStat(0);
  
  //int rebin = 8;

  mat->GetXaxis()->UnZoom();
  mat->GetYaxis()->UnZoom();

  double arr[] = {bgl1, bgr1, bgl2, bgr2};
  std::sort(arr,arr+4);
  bgl1 = arr[0]; 
  bgr1 = arr[1]; 
  bgl2 = arr[2]; 
  bgr2 = arr[3]; 
 
  if(bgl1<0 || bgr1<0){
    bgr1 = lower;
    bgl1 = bgr1 - (upper-lower)/2;
  }
  if(bgl2<0 || bgr2<0){
    bgl2 = upper;
    bgr2 = bgl2 + (upper-lower)/2;
  }

  int lower_bin = mat->GetXaxis()->FindBin(lower);
  int upper_bin = mat->GetXaxis()->FindBin(upper)-1;
  int bgl1_bin  = mat->GetXaxis()->FindBin(bgl1);
  int bgr1_bin  = mat->GetXaxis()->FindBin(bgr1)-1;
  int bgl2_bin  = mat->GetXaxis()->FindBin(bgl2);
  int bgr2_bin  = mat->GetXaxis()->FindBin(bgr2)-1;
  
  TH1D *px = mat->ProjectionX(Form("%s_px",mat->GetName()),lower_bin,upper_bin);
  px->SetTitle(Form("%s projected [%.1f, %.1f]keV in bin[%i,%i]", mat->GetTitle(), lower, upper, lower_bin, upper_bin));  
  px->Rebin(rebin);
  
  TH1D *bg1 = mat->ProjectionX(Form("%s_bg1",mat->GetName()),bgl1_bin,bgr1_bin);
  bg1->Rebin(rebin);
  TH1D *bg2 = mat->ProjectionX(Form("%s_bg2",mat->GetName()),bgl2_bin,bgr2_bin);
  bg2->Rebin(rebin);
  TH1D *bg = (TH1D *)bg1->Clone(Form("%s_bg",mat->GetName()));
  bg->Add(bg2);
  bg->SetTitle(Form("%s bg [%.1f, %.1f] + [%.1f, %.1f]", mat->GetTitle(),bgl1,bgr1,bgl2,bgr2));

  TH1D *cpx = (TH1D *)px->Clone(Form("%s_cl",px->GetName()));
  cpx->Add(bg,-1);
  cpx->SetTitle(Form("%s projected[%.1f, %.1f] subbg [%.1f, %.1f] & [%.1f, %.1f] (bin: [%i,%i] - [%i,%i]&[%i,%i])"
                      , mat->GetTitle(),lower,upper,bgl1,bgr1,bgl2,bgr2
                                       ,lower_bin,upper_bin, bgl1_bin,bgr1_bin, bgl2_bin, bgr2_bin));
  cpx->SetMinimum(0);

  TH1D *tot = mat->ProjectionX(Form("%s_tot",mat->GetName()));
  tot->SetTitle(Form("%s total projection", mat->GetTitle()));
  tot->GetXaxis()->SetRangeUser(bgl1-4, bgr2+4);

  double ymax = tot->GetMaximum();
  TLine *l1   = new TLine(lower,0 ,lower,ymax);
  TLine *l2   = new TLine(upper,0 ,upper,ymax);
  TLine *lbgl1= new TLine(bgl1 ,0 ,bgl1 ,ymax);
  TLine *lbgr1= new TLine(bgr1 ,0 ,bgr1 ,ymax);
  TLine *lbgl2= new TLine(bgl2 ,0 ,bgl2 ,ymax);
  TLine *lbgr2= new TLine(bgr2 ,0 ,bgr2 ,ymax);
  l1   ->SetLineWidth(2);
  l2   ->SetLineWidth(2);
  l1   ->SetLineColor(kRed);
  l2   ->SetLineColor(kRed);
  lbgl1->SetLineColor(kGreen);
  lbgr1->SetLineColor(kGreen);
  lbgl2->SetLineColor(kGreen);
  lbgr2->SetLineColor(kGreen);

  //px ->GetXaxis()->SetRangeUser(650, 2000);
  //cpx->GetXaxis()->SetRangeUser(650, 2000);
  
  TCanvas *c = new TCanvas();
  c->Divide(1,3);
  c->cd(1);tot->Draw("hist");
           l1   ->Draw("same");l2   ->Draw("same");
           lbgl1->Draw("same");lbgr1->Draw("same");
           lbgl2->Draw("same");lbgr2->Draw("same");
  c->cd(2);px ->Draw("hist");
           bg->Draw("same hist");
  c->cd(3);cpx->Draw("hist");

  px->SetLineWidth(2);
  bg->SetLineColor(kRed);

}
