// c++ $(root-config --cflags) -Iinclude macros/cxx/physicstree_promptgood2.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lS2426PHYSICS -lTMIDAS -lCHANNEL $(root-config --libs) -o macros/cxx/bin/physicstree_promptgood2

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
#include <numeric>
#include <utility>

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
  TFile *cutfile3 = TFile::Open("TCutG/physicstree_promptgood/SIIC_banana.root");
  TCutG *siic_cut[21];
  for(int m=0;m<21;m++){
    siic_cut[m] = (TCutG *)cutfile3->Get(Form("cutg%i",m));
  }
  TFile *cutfile4 = TFile::Open("TCutG/physicstree_promptgood/ic0_ic3/IC3IC0_banana.root");
  TCutG *ic3ic0_cut[12];
  for(int m=0;m<12;m++){
    ic3ic0_cut[m] = (TCutG *)cutfile4->Get(Form("cutg%i",m));
  }
  // ======== Par Setup ========= //
  double beta = 0.056;
  double pgacxg[3][2] = {{-18,-13},
                         {-10,-5},
                         {3,8}}; 

  long nentries = tree->GetEntries();
  long xentry = 0;
  for(xentry = 0; xentry < nentries; xentry++) {
    tree->GetEntry(xentry);
    if(emma->Left().size() == 0 || emma->Right().size() == 0) {
      for(int i=0;i<tig->Hits().size();i++){          
        TigressHit tighit = tig->Hits()[i];           
        double e1 = tighit.Doppler(beta);             
        long t1   = tighit.TimestampNs();             
        int arry1 = tighit.ArrayNumber();             
        if(e1<15) continue;      
        if(tighit.KValue()!=379) continue;            
        if(tighit.BGOFire()) continue;                
        Histogramer::Fill("PromptGood_singleAnode", " Energy vs ArrayNumber",                  64,0,64,arry1,8e3,0,8e3,tighit.Energy());
        Histogramer::Fill("PromptGood_singleAnode", Form(" Doppler(%.3f) vs ArrayNumber",beta),64,0,64,arry1,8e3,0,8e3,e1);
      }
      continue;
    }
    double pgacx = emma->PGACX();
    double ics[5] = {0,0,0,0,0};
    std::pair<int,double> lastIC = {-1,0.0};
    if(!emma->IC0().empty() && emma->IC0()[0].Charge()<3500) {ics[0] = emma->IC0()[0].Charge(); lastIC = {0,emma->IC0()[0].Charge()};}
    if(!emma->IC1().empty() && emma->IC1()[0].Charge()<3500) {ics[1] = emma->IC1()[0].Charge(); lastIC = {1,emma->IC1()[0].Charge()};}
    if(!emma->IC2().empty() && emma->IC2()[0].Charge()<3500) {ics[2] = emma->IC2()[0].Charge(); lastIC = {2,emma->IC2()[0].Charge()};}
    if(!emma->IC3().empty() && emma->IC3()[0].Charge()<3500) {ics[3] = emma->IC3()[0].Charge(); lastIC = {3,emma->IC3()[0].Charge()};}
    if(!emma->Si().empty()  && emma->Si()[0].Charge()<3500)  {ics[4] = emma->Si()[0].Charge();  lastIC = {4,emma->Si()[0].Charge()};}
  
// ========== check abnormal =============== //
    bool hit[5] = { !emma->IC0().empty() && emma->IC0()[0].Charge()<3500,
                    !emma->IC1().empty() && emma->IC1()[0].Charge()<3500,
                    !emma->IC2().empty() && emma->IC2()[0].Charge()<3500,
                    !emma->IC3().empty() && emma->IC3()[0].Charge()<3500,
                    !emma->Si().empty()  && emma->Si()[0].Charge()<3500};
    int missing = -1;
    for(int i=0;i<5;i++){
      if(!hit[i]) missing = i;
      if(missing>=0 && hit[i]) {
        Histogramer::Fill("PID","Abnomal ions in EMMA", 10,0,10, missing);
        break;
      }
    } 
// ========== check abnormal (over)=============== //
    double sum = std::accumulate(ics, ics+5, 0.0);
    if(lastIC.first>0) Histogramer::Fill("PID/SUM",Form("IC%i vs Sum", lastIC.first),2.5e3,0,10e3,sum, 1e3,0,4e3,lastIC.second);
    if(!emma->Si().empty() && !emma->IC0().empty()){ 
      Histogramer::Fill("PID/ICs","IC0 vs Si", 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[0]);
    }
    if(!emma->Si().empty() && !emma->IC1().empty()) {
      Histogramer::Fill("PID/ICs","IC1 vs Si", 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[1]);
    }
    if(!emma->Si().empty() && !emma->IC2().empty()) {
      Histogramer::Fill("PID/ICs","IC2 vs Si", 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[2]);
    }
    if(!emma->Si().empty() && !emma->IC3().empty()){ 
      Histogramer::Fill("PID/ICs","IC3 vs Si", 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[3]);
    }
    if(!emma->IC3().empty() && !emma->IC0().empty()){ 
      Histogramer::Fill("PID/ICs","IC0 vs IC3", 1e3,0,4e3,ics[3], 1e3,0,4e3,ics[0]);
    }
    if(!emma->IC3().empty() && !emma->IC0().empty() && emma->Si().empty()){ 
      Histogramer::Fill("PID/ICs","IC0 vs IC3i when Si.empty", 1e3,0,4e3,ics[3], 1e3,0,4e3,ics[0]);
    }
    // ==== pgacx gated PID ==== //
    if(pgacx>-20 && pgacx<10){
      int pgacx_int = (int)pgacx;    
      if(lastIC.first>0) Histogramer::Fill(Form("PID/PGACXGate/%i/SUM",pgacx_int),Form("IC%i vs Sum gated pgacx=[%i,%i)",lastIC.first,pgacx_int, pgacx_int+1),2.5e3,0,10e3,sum, 1e3,0,4e3,lastIC.second); 
      if(!emma->Si().empty() && !emma->IC0().empty()){ 
        Histogramer::Fill(Form("PID/PGACXGate/%i/ICs",pgacx_int),Form("IC0 vs Si gated pgacx=[%i,%i)",pgacx_int, pgacx_int+1), 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[0]); 
      }
      if(!emma->Si().empty() && !emma->IC1().empty()) {
        Histogramer::Fill(Form("PID/PGACXGate/%i/ICs",pgacx_int),Form("IC1 vs Si gated pgacx=[%i,%i)",pgacx_int, pgacx_int+1), 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[1]); 
      }
      if(!emma->Si().empty() && !emma->IC2().empty()) {
        Histogramer::Fill(Form("PID/PGACXGate/%i/ICs",pgacx_int),Form("IC2 vs Si gated pgacx=[%i,%i)",pgacx_int, pgacx_int+1), 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[2]); 
      }
      if(!emma->Si().empty() && !emma->IC3().empty()){ 
        Histogramer::Fill(Form("PID/PGACXGate/%i/ICs",pgacx_int),Form("IC3 vs Si gated pgacx=[%i,%i)",pgacx_int, pgacx_int+1), 1e3,0,4e3,ics[4], 1e3,0,4e3,ics[3]); 
      }
      if(!emma->IC3().empty() && !emma->IC0().empty()){ 
        Histogramer::Fill(Form("PID/PGACXGate/%i/ICs",pgacx_int),Form("IC0 vs IC3 gated pgacx=[%i,%i)",pgacx_int, pgacx_int+1), 1e3,0,4e3,ics[3], 1e3,0,4e3,ics[0]); 
      }
      if(!emma->IC3().empty() && !emma->IC0().empty() && emma->Si().empty()){ 
        Histogramer::Fill(Form("PID/PGACXGate/%i/ICs",pgacx_int),Form("IC0 vs IC3 gated pgacx=[%i,%i) whem Si.empty",pgacx_int, pgacx_int+1), 1e3,0,4e3,ics[3], 1e3,0,4e3,ics[0]); 
      }
    }
    // ==== TIG i loop start ==== // 
    for(int i=0;i<tig->Hits().size();i++){
      TigressHit tighit = tig->Hits()[i];
      double e1 = tighit.Doppler(beta);
      long t1   = tighit.TimestampNs();
      int arry1 = tighit.ArrayNumber();
      if(e1<15) continue;
      if(tighit.KValue()!=379) continue;
      if(tighit.BGOFire()) continue;
      Histogramer::Fill("PromptGoodTree", " Energy vs ArrayNumber",                  64,0,64,arry1,8e3,0,8e3,tighit.Energy());
      Histogramer::Fill("PromptGoodTree", Form(" Doppler(%.3f) vs ArrayNumber",beta),64,0,64,arry1,8e3,0,8e3,e1);
      if(arry1<48) Histogramer::Fill("TIG", Form("PGACX vs Doppler(%.3f) mid ring",beta),60,-30,30,pgacx,4e3,0,4e3,e1); 
      // ====== ics vs si gate starts ===== //
      if(ics[4]){
        for(int m=0;m<4;m++){ // loop 4 ic
          if(!ics[m]) continue;
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
      if(ics[0] && ics[3]){ // if IC0 and IC3 not empty
        for(m=0;m<12;m++){ // ic0 vs ic3 cut loop
          if(ic3ic0_cut[m]->IsInsie(ics[3],ics[0])){
            if(arr1<48) Histogramer::Fill("TIG/IC3_IC0_Gate", Form("PGACX vs Doppler(%.3f) gated ic3 vs ic0 %s mid ring",beta),60,-30,30,pgacx,4e3,0,4e3,e1);
          }
        }// ic0 vs ic3 cut over
      }// if IC0 and IC3 no empty over




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
          // ====== ics vs si gate starts ===== //
          if(ics[4]){
            for(int m=0;m<4;m++){ // loop 4 ic
            if(!ics[m]) continue;
              for(int n=0;n<=20;n++){ // loop 21 tcutg
                if(siic_cut[n]->IsInside(ics[4],ics[m])){
                  if(arry1<48 && arry2<48){
                    if(ics[4]<150){
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              2000,0,4000,e1,2000,0,4000,e2);
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              2000,0,4000,e2,2000,0,4000,e1);
                    }else{
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              2000,0,4000,e1,2000,0,4000,e2);
                      Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),Form("gg matrix: Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",beta,m,siic_cut[n]->GetName()),
                                              2000,0,4000,e2,2000,0,4000,e1);
                    }
                  }
                  for(int k=0;k<3;k++){ // pgacx gate                                                                                         
                    if(pgacx>=pgacxg[k][0] && pgacx<=pgacxg[k][1]){
                      if(arry1<48 && arry2<48){
                        if(ics[4]<150){
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            2000,0,4000,e1,2000,0,4000,e2);           
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Left/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            2000,0,4000,e2,2000,0,4000,e1);
                        }else{
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            2000,0,4000,e1,2000,0,4000,e2);           
                          Histogramer::Fill(Form("TIG/Coinc/ICs_Si_Gate/IC%i/Right/mid ring",m),
                                            Form("gg matrix:PGACX=[%.0f,%.0f]:Doppler(%.3f) within dtns=[-50,150]ns gated ic%i vs si %s mid ring",pgacxg[k][0],pgacxg[k][1],beta,m,siic_cut[n]->GetName()),
                                            2000,0,4000,e2,2000,0,4000,e1);
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

// ============== Sort PromptBadTree =========== //
  TTree* tree2 = (TTree*)infile->Get("PromptBadTree");
  if(!tree2) {
    std::printf("PromptBadTree not found\n");
    infile->Close();
    return 1;
  }

  Tigress* tig2 = nullptr;
  Emma* emma2 = nullptr;
  tree2->SetBranchAddress("Tigress", &tig2);
  tree2->SetBranchAddress("Emma", &emma2);

  long nentries2 = tree2->GetEntries();
  long xentry2 = 0;
  for(xentry2 = 0; xentry2 < nentries2; xentry2++) {
    tree2->GetEntry(xentry2);
    for(int i=0;i<tig2->Hits().size();i++){          
      TigressHit tighit = tig2->Hits()[i];           
      double e1 = tighit.Doppler(beta);             
      long t1   = tighit.TimestampNs();             
      int arry1 = tighit.ArrayNumber();             
      if(e1<15) continue;      
      if(tighit.KValue()!=379) continue;            
      if(tighit.BGOFire()) continue;                
      Histogramer::Fill("PromptBadTree", " Energy vs ArrayNumber",                  64,0,64,arry1,8e3,0,8e3,tighit.Energy());
      Histogramer::Fill("PromptBadTree", Form(" Doppler(%.3f) vs ArrayNumber",beta),64,0,64,arry1,8e3,0,8e3,e1);
    }
    if((xentry2%5000)==0){
      printf("on entry = %lu / %lu \r", xentry2, nentries2);
      fflush(stdout);
    }
  }
  printf("on entry = %lu / %lu \n", xentry2, nentries2);

// ============== Sort BgTree =========== //
  TTree* tree3 = (TTree*)infile->Get("BgTree");
  if(!tree3) {
    std::printf("BgTree not found\n");
    infile->Close();
    return 1;
  }

  Tigress* tig3 = nullptr;
  Emma* emma3 = nullptr;
  tree3->SetBranchAddress("Tigress", &tig3);
  tree3->SetBranchAddress("Emma", &emma3);

  long nentries3 = tree3->GetEntries();
  long xentry3 = 0;
  for(xentry3 = 0; xentry3 < nentries3; xentry3++) {
    tree3->GetEntry(xentry3);
    for(int i=0;i<tig3->Hits().size();i++){          
      TigressHit tighit = tig3->Hits()[i];           
      double e1 = tighit.Doppler(beta);             
      long t1   = tighit.TimestampNs();             
      int arry1 = tighit.ArrayNumber();             
      if(e1<15) continue;      
      if(tighit.KValue()!=379) continue;            
      if(tighit.BGOFire()) continue;                
      Histogramer::Fill("BgTree", " Energy vs ArrayNumber",                  64,0,64,arry1,8e3,0,8e3,tighit.Energy());
      Histogramer::Fill("BgTree", Form(" Doppler(%.3f) vs ArrayNumber",beta),64,0,64,arry1,8e3,0,8e3,e1);
    }
    if((xentry2%5000)==0){
      printf("on entry = %lu / %lu \r", xentry2, nentries2);
      fflush(stdout);
    }
  }
  printf("on entry = %lu / %lu \n", xentry2, nentries2);
// ============================================= //

  Histogramer::Close();
  infile->Close();
  delete infile;

  return 0;
}
