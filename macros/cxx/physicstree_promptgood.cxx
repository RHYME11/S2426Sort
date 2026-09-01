// c++ $(root-config --cflags) -Iinclude macros/cxx/physicstree_promptgood.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lS2426PHYSICS -lTMIDAS -lCHANNEL $(root-config --libs) -o macros/cxx/bin/physicstree_promptgood

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
#include <numeric>

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <TCutG.h>
#include <Channel.h>
#include <Emma.h>
#include <Histogramer.h>
#include <Tigress.h>

// ============== main ==============
// Purpose: Fill histograms from PromptGoodTree.
// Inputs: Physics ROOT file path.
// Outputs: Histogram ROOT file named from the input basename.
int main(int argc, char** argv) {
  if(argc != 2) {
    std::printf("usage: %s path/to/physics<run>_<subrun>.root\n", argv[0]);
    return 1;
  }

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");

  const std::filesystem::path inputPath(argv[1]);
  const std::string inputName = inputPath.filename().string();
  TFile* infile = TFile::Open(inputPath.c_str());
  if(!infile || infile->IsZombie()) {
    std::printf("failed to open %s\n", inputPath.c_str());
    return 1;
  }

  TTree* tree = (TTree*)infile->Get("PromptGoodTree");
  if(!tree) {
    std::printf("PromptGoodTree not found\n");
    infile->Close();
    return 1;
  }

  Tigress* tig = nullptr;
  Emma* emma = nullptr;
  tree->SetBranchAddress("Tigress", &tig);
  tree->SetBranchAddress("Emma", &emma);

  const std::string outputPath =
    "histOutput/physicstree_promptgood/hist_" + inputName;
  Histogramer::Get()->SetOutputPath(outputPath);

  // ======== TCutG Load ======== //
  TFile *cutfile0 = TFile::Open("TCutG/physicstree_promptgood/ic0_si/SiIC0_gamgated_banana.root");
  TCutG *siic0_cut[2];
  siic0_cut[0] = (TCutG *)cutfile0->Get("siic0_top");
  siic0_cut[1] = (TCutG *)cutfile0->Get("siic0_bot");
  TFile *cutfile1 = TFile::Open("TCutG/physicstree_promptgood/ic0_si/SiIC0_banana.root");
  TCutG *siic0_cut2[7];
  for(int m=0;m<7;m++){
    siic0_cut2[m] = (TCutG *)cutfile1->Get(Form("cutg%i",m));
  }
  TFile *cutfile2 = TFile::Open("TCutG/physicstree_promptgood/ic2_si/SiIC2_banana.root");
  TCutG *siic2_cut[3][7];
  for(int m=0;m<7;m++){
    siic2_cut[0][m] = (TCutG *)cutfile2->Get(Form("lic2si_cutg%i",m));
    if(m==6) continue;
    siic2_cut[1][m] = (TCutG *)cutfile2->Get(Form("mic2si_cutg%i",m));
    siic2_cut[2][m] = (TCutG *)cutfile2->Get(Form("ric2si_cutg%i",m));
  }
  TFile *cutfile3 = TFile::Open("TCutG/physicstree_promptgood/SIIC_banana.root");
  TCutG *siic_cut[21];
  for(int m=0;m<21;m++){
    siic_cut[m] = (TCutG *)cutfile3->Get(Form("cutg%i",m));
  }
  // ======== Par Setup ========= //
  double beta = 0.056;
  double pgacxg[3][2] = {{-18,-13},
                         {-10,-5},
                         {3,8}}; 

  std::vector<double> ics;

  long nentries = tree->GetEntries();
  long xentry = 0;
  for(xentry = 0; xentry < nentries; xentry++) {
    tree->GetEntry(xentry);
    if(emma->Left().size() == 0 || emma->Right().size() == 0) continue;

    double pgacx = emma->PGACX();
    if(!emma->IC0().empty()) ics.push_back(emma->IC0()[0].Charge());
    if(!emma->IC1().empty()) ics.push_back(emma->IC1()[0].Charge());
    if(!emma->IC2().empty()) ics.push_back(emma->IC2()[0].Charge());
    if(!emma->IC3().empty()) ics.push_back(emma->IC3()[0].Charge());
    if(!emma->Si().empty())  ics.push_back(emma->Si()[0].Charge());
  
// ========== check abnormal =============== //
    bool hit[5] = { !emma->IC0().empty(),
                    !emma->IC1().empty(),
                    !emma->IC2().empty(),
                    !emma->IC3().empty(),
                    !emma->Si().empty()};
    int missing = -1;
    for(int i=0;i<5;i++){
      if(!hit[i]) missing = i;
      if(missing>0 && hit[i]) {
        Histogramer::Fill("PID","Abnomal ions in EMMA", 10,0,10, missing);
        break;
      }
    } 
// ========== check abnormal (over)=============== //
    double sum = std::accumulate(ics.begin(), ics.end(), 0.0);
    if(!ics.empty() && ics.size()<5) Histogramer::Fill("PID/SUM",Form("IC%i vs Sum(no si)",ics.size()-1),4e3,0,4e3,sum, 4e4,0,4e3,ics.back());
    else if (ics.size() == 5)        Histogramer::Fill("PID/SUM",     "IC3 vs Sum(has si)"              ,4e3,0,4e3,sum, 4e4,0,4e3,ics[3]);
    if(!emma->Si().empty() && !emma->IC0().empty()){ 
      Histogramer::Fill("PID/IC0","IC0 vs Si", 4e3,0,4e3,ics[4], 4e3,0,4e3,ics[0]);
    }
    if(!emma->Si().empty() && !emma->IC1().empty()) {
      Histogramer::Fill("PID/IC1","IC1 vs Si", 4e3,0,4e3,ics[4], 4e3,0,4e3,ics[1]);
    }
    if(!emma->Si().empty() && !emma->IC2().empty()) {
      Histogramer::Fill("PID/IC2","IC2 vs Si", 4e3,0,4e3,ics[4], 4e3,0,4e3,ics[2]);
    }
    if(!emma->Si().empty() && !emma->IC3().empty()){ 
      Histogramer::Fill("PID/IC3","IC3 vs Si", 4e3,0,4e3,ics[4], 4e3,0,4e3,ics[3]);
    }
    
    // ==== TIG i loop start ==== // 
    for(int i=0;i<tig->Hits().size();i++){
      TigressHit tighit = tig->Hits()[i];
      double e1 = tighit.Doppler(beta);
      long t1   = tighit.TimestampNs();
      int arry1 = tighit.ArrayNumber();
      if(e1<15) continue;
      if(tighit.KValue()!=379) continue;
      Histogramer::Fill("TIG","no BGO veto: summary single",                  64,0,64,arry1,4e3,0,4e3,e1);
      Histogramer::Fill("TIG",Form("no BGO veto: summary Doppler(%.3f)",beta),64,0,64,arry1,4e3,0,4e3,e1);
      if(tighit.BGOFire()) continue;
      Histogramer::Fill("TIG",Form("summary Doppler(%.3f)",beta) , 64,0,64,arry1  ,4e3,0,4e3,e1);
      Histogramer::Fill("TIG",Form("PGACX vs Doppler(%.3f)",beta), 60,-30,30,pgacx,4e3,0,4e3,e1);
      if(arry1<48) {
        Histogramer::Fill("TIG",Form("PGACX vs Doppler(%.3f) mid ring",beta), 60,-30,30,pgacx,4e3,0,4e3,e1);
      }
      // ====== is0 vs si gate starts ===== //
      if(!emma->Si().empty() && !emma->IC0().empty()){
        for(int m=0;m<2;m++){
          if(siic0_cut[m]->IsInside(ics[4], ics[0])){
            if(arry1<48) {
              Histogramer::Fill("TIG/IC0_Si_Gate",Form("PGACX vs Doppler(%.3f) gated %s mid ring",beta,siic0_cut[m]->GetName()),60,-30,30,pgacx,4e3,0,4e3,e1);
            }
            Histogramer::Fill("TIG/IC0_Si_Gate", Form("summary Doppler(%.3f) gated %s",beta,siic0_cut[m]->GetName()),64,0,64,arry1  ,4e3,0,4e3,e1);
          }
        }// gate loop over
        for(int m=0;m<7;m++){
          if(siic0_cut2[m]->IsInside(ics[4], ics[0])){
            if(arry1<48) Histogramer::Fill("TIG/IC0_Si_Gate",Form("PGACX vs Doppler(%.3f) gated %s mid ring",beta,siic0_cut2[m]->GetName()),60,-30,30,pgacx,4e3,0,4e3,e1);
            Histogramer::Fill("TIG/IC0_Si_Gate", Form("summary Doppler(%.3f) gated %s",beta,siic0_cut2[m]->GetName()),64,0,64,arry1  ,4e3,0,4e3,e1);
            for(int n=0;n<3;n++){
              if(pgacx>=pgacxg[n][0] && pgacx<=pgacxg[n][1]){
                if(arry1<48) {
                  Histogramer::Fill("TIG/IC0_Si_Gate",Form("PGACX=[%.0f,%.0f]: summary Doppler(%.3f) gated %s mid ring",pgacxg[n][0],pgacxg[n][1],beta,siic0_cut2[m]->GetName()),64,0,64,arry1,4e3,0,4e3,e1);
                }
              }
            }// pgacx gate loop over
          }// if cutg gate over
        }// gate loop over
      } // ic0 vs si gate over
      // ====== ic2 vs si gate starts ===== //
      if(!emma->Si().empty() && !emma->IC2().empty()){
        for(int m=0;m<3;m++){
          for(int n=0;n<7;n++){
            if(m>0 && n==6) continue; // only 6 cuts for m=1 and 2
            if(siic2_cut[m][n]->IsInside(ics[4],ics[2])){
              if(arry1<48) {
                Histogramer::Fill("TIG/IC2_Si_Gate",Form("PGACX vs Doppler(%.3f) gated %s mid ring",beta,siic2_cut[m][n]->GetName()),60,-30,30,pgacx,4e3,0,4e3,e1);
              }
              Histogramer::Fill("TIG/IC2_Si_Gate", Form("summary Doppler(%.3f) gated %s",beta,siic2_cut[m][n]->GetName()),64,0,64,arry1  ,4e3,0,4e3,e1);
              for(int k=0;k<3;k++){ // pgacx gate
                if(pgacx>=pgacxg[k][0] && pgacx<=pgacxg[k][1]){
                  if(arry1<48) {
                    Histogramer::Fill("TIG/IC2_Si_Gate",Form("PGACX=[%.0f,%.0f]: summary Doppler(%.3f) gated %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,siic2_cut[m][n]->GetName()),64,0,64,arry1,4e3,0,4e3,e1);
                  }
                }
              } // pgacx gate loop over
            }
          }// n loop over
        }// m loop over
      }// ic2 vs si gate over
      // ====== ics vs si gate starts ===== //
      if(!emma->Si().empty()>0){
        for(int m=0;m<4;m++){ // loop 4 ic
          for(int n=0;n<=20;n++){ // loop 21 tcutg
            if(siic_cut[n]->IsInside(ics[4],ics[m])){
              if(arry1<48){
                if(ics[4]<150) Histogramer::Fill(Form("TIG/ICs_Si_Gate/IC%i/Left",m), Form("PGACX vs Doppler(%.3f) gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),60,-30,30,pgacx,4e3,0,4e3,e1);
                else           Histogramer::Fill(Form("TIG/ICs_Si_Gate/IC%i/Right",m),Form("PGACX vs Doppler(%.3f) gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),60,-30,30,pgacx,4e3,0,4e3,e1);
              }
              if(ics[4]<150) Histogramer::Fill(Form("TIG/ICs_Si_Gate/IC%i/Left",m),  Form("summary Doppler(%.3f) gated ic%i vs si %s",beta,m,siic_cut[n]->GetName()),64,0,64,arry1  ,4e3,0,4e3,e1);
              else           Histogramer::Fill(Form("TIG/ICs_Si_Gate/IC%i/Right",m), Form("summary Doppler(%.3f) gated ic%i vs si %s",beta,m,siic_cut[n]->GetName()),64,0,64,arry1  ,4e3,0,4e3,e1);
            }        
          }// 21 tcutg loop over
        }// 4 ic loop over
      }// if si no empty over



      // ==== coincidence start ==== //
      for(int j=i+1;j<tig->Hits().size();j++){
        TigressHit tighit2 = tig->Hits()[j];
        double e2 = tighit2.Doppler(beta);
        long t2   = tighit2.TimestampNs();   
        int arry2 = tighit2.ArrayNumber();
        if(e2<15) continue;
        if(tighit2.KValue()!=379) continue;
        if(tighit2.BGOFire()) continue;
        double dtns;
        double e;
        if(e1>e2){
          dtns = t2-t1;
          e = e1;
        }else{
          dtns = t1 -t2;
          e = e2;
        }
        Histogramer::Fill("TIG/Coinc",Form("dt(TimestampNs) vs Higher Dopper(b=%.3f)",beta),6e2,-3e3,3e3, dtns, 4000,0,4000,e);
        if(dtns>=-50 && dtns<=150){ 
          Histogramer::Fill("TIG/Coinc",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns",beta),4000,0,4000,e1,4000,0,4000,e2);
          Histogramer::Fill("TIG/Coinc",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns",beta),4000,0,4000,e2,4000,0,4000,e1);
          // ====== is0 vs si gate starts ===== //
          if(!emma->Si().empty() && !emma->IC0().empty()){
            for(int m=0;m<7;m++){
              if(siic0_cut2[m]->IsInside(ics[4], ics[0])){
                Histogramer::Fill("TIG/Coinc/IC0_Si_Gate",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s",beta,siic0_cut2[m]->GetName()),4000,0,4000,e1,4000,0,4000,e2);
                Histogramer::Fill("TIG/Coinc/IC0_Si_Gate",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s",beta,siic0_cut2[m]->GetName()),4000,0,4000,e2,4000,0,4000,e1);
                if(arry1<48 && arry2<48){
                  Histogramer::Fill("TIG/Coinc/IC0_Si_Gate/mid ring",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",beta,siic0_cut2[m]->GetName()),4000,0,4000,e1,4000,0,4000,e2);
                  Histogramer::Fill("TIG/Coinc/IC0_Si_Gate/mid ring",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",beta,siic0_cut2[m]->GetName()),4000,0,4000,e2,4000,0,4000,e1);
                }
                for(int n=0;n<3;n++){
                  if(pgacx>=pgacxg[n][0] && pgacx<=pgacxg[n][1]){
                    Histogramer::Fill("TIG/Coinc/IC0_Si_Gate",Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s",pgacxg[n][0],pgacxg[n][1],beta,siic0_cut2[m]->GetName()),
                                      4000,0,4000,e1,4000,0,4000,e2);
                    Histogramer::Fill("TIG/Coinc/IC0_Si_Gate",Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s",pgacxg[n][0],pgacxg[n][1],beta,siic0_cut2[m]->GetName()),
                                      4000,0,4000,e2,4000,0,4000,e1);
                    if(arry1<48 && arry2<48){
                      Histogramer::Fill("TIG/Coinc/IC0_Si_Gate/mid ring",
                                        Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",pgacxg[n][0],pgacxg[n][1],beta,siic0_cut2[m]->GetName()),
                                        4000,0,4000,e1,4000,0,4000,e2);
                      Histogramer::Fill("TIG/Coinc/IC0_Si_Gate/mid ring",
                                        Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",pgacxg[n][0],pgacxg[n][1],beta,siic0_cut2[m]->GetName()),
                                        4000,0,4000,e2,4000,0,4000,e1);
                    }
                  }
                }
              } // if ic0 vs si gate over
            }// ic0 vs si gate (m) loop over
          }// ic0 vs si gate over
          // ====== is2 vs si gate starts ===== //
          if(!emma->Si().empty() && !emma->IC2().empty()){
            for(int m=0;m<3;m++){
              for(int n=0;n<7;n++){
                if(m>0 && n==6) continue; // only 6 cuts for m=1 and 2
                if(siic2_cut[m][n]->IsInside(ics[4],ics[2])){
                  Histogramer::Fill("TIG/Coinc/IC2_Si_Gate",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s",beta,siic2_cut[m][n]->GetName()),4000,0,4000,e1,4000,0,4000,e2);
                  Histogramer::Fill("TIG/Coinc/IC2_Si_Gate",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s",beta,siic2_cut[m][n]->GetName()),4000,0,4000,e2,4000,0,4000,e1);
                  if(arry1<48 && arry2<48){
                    Histogramer::Fill("TIG/Coinc/IC2_Si_Gate/mid ring",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",beta,siic2_cut[m][n]->GetName()),
                                      4000,0,4000,e1,4000,0,4000,e2);
                    Histogramer::Fill("TIG/Coinc/IC2_Si_Gate/mid ring",Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",beta,siic2_cut[m][n]->GetName()),
                                      4000,0,4000,e2,4000,0,4000,e1);
                  }
                  for(int k=0;k<3;k++){ // pgacx gate
                    if(pgacx>=pgacxg[k][0] && pgacx<=pgacxg[k][1]){
                      Histogramer::Fill("TIG/Coinc/IC2_Si_Gate",Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s",pgacxg[k][0],pgacxg[k][1],beta,siic2_cut[m][n]->GetName()),
                                        4000,0,4000,e1,4000,0,4000,e2);
                      Histogramer::Fill("TIG/Coinc/IC2_Si_Gate",Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s",pgacxg[k][0],pgacxg[k][1],beta,siic2_cut[m][n]->GetName()),
                                        4000,0,4000,e2,4000,0,4000,e1);
                      if(arry1<48 && arry2<48){
                        Histogramer::Fill("TIG/Coinc/IC2_Si_Gate/mid ring",
                                          Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,siic2_cut[m][n]->GetName()),
                                          4000,0,4000,e1,4000,0,4000,e2);
                        Histogramer::Fill("TIG/Coinc/IC2_Si_Gate/mid ring",
                                          Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,siic2_cut[m][n]->GetName()),
                                          4000,0,4000,e2,4000,0,4000,e1);
                      }
                    }
                  } // pgacx gate loop over
                }
              }// n loop over
            }// m loop over
          }// ic2 vs si gate over
          // ====== ics vs si gate starts ===== //
          if(!emma->Si().empty()>0){
            for(int m=0;m<4;m++){ // loop 4 ic
              for(int n=0;n<=20;n++){ // loop 21 tcutg
                if(siic_cut[n]->IsInside(ics[4],ics[m])){
                  if(ics[4]<150){
                    Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e1,4000,0,4000,e2);
                    Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e2,4000,0,4000,e1);
                  }else{
                    Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e1,4000,0,4000,e2);
                    Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e2,4000,0,4000,e1);
                  }
                  if(arry1<48 && arry2<48){
                    if(ics[4]<150){
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              4000,0,4000,e1,4000,0,4000,e2);
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              4000,0,4000,e2,4000,0,4000,e1);
                    }else{
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              4000,0,4000,e1,4000,0,4000,e2);
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              4000,0,4000,e2,4000,0,4000,e1);
                    }
                  }
                  for(int k=0;k<3;k++){ // pgacx gate                                                                                         
                    if(pgacx>=pgacxg[k][0] && pgacx<=pgacxg[k][1]){
                      if(ics[4]<150){
                        Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/Left/IC%i",m),
                                          Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                          4000,0,4000,e1,4000,0,4000,e2);           
                        Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/Left/IC%i",m),
                                          Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                          4000,0,4000,e2,4000,0,4000,e1);
                      }else{
                        Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/Right/IC%i",m),
                                          Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                          4000,0,4000,e1,4000,0,4000,e2);           
                        Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/Right/IC%i",m),
                                          Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                          4000,0,4000,e2,4000,0,4000,e1);
                      }
                      if(arry1<48 && arry2<48){
                        if(ics[4]<150){
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e1,4000,0,4000,e2);           
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e2,4000,0,4000,e1);
                        }else{
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e1,4000,0,4000,e2);           
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            4000,0,4000,e2,4000,0,4000,e1);
                        }
                      }// if array number over
                    } // if pgacx over
                  } // pgacx gate loop over
                } // if tcug over
              } // 21 tcutg loop over
            } // 4 ic loop over
          } // if si no empty over
        }// if dtns over
      }// loop j over
    }// loop i over


    if((xentry%5000)==0){
      printf("on entry = %lu / %lu \r", xentry, nentries);
      fflush(stdout);
    }
  } // tree loop over

  printf("on entry = %lu / %lu \n", xentry, nentries);


  Histogramer::Close();
  infile->Close();
  delete infile;

  return 0;
}
