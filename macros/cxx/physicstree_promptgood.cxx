// c++ $(root-config --cflags) -Iinclude macros/cxx/physicstree_promptgood.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lS2426PHYSICS -lTMIDAS -lCHANNEL $(root-config --libs) -o macros/cxx/bin/physicstree_promptgood

#include <cstdio>
#include <filesystem>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
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

  long nentries = tree->GetEntries();
  long xentry = 0;
  double beta = 0.056;
  for(xentry = 0; xentry < nentries; xentry++) {
    tree->GetEntry(xentry);
    if(emma->Left().size() == 0 || emma->Right().size() == 0)
      continue;

    Histogramer::Fill("EMMA", "Si.size()",  10, 0, 10, emma->Si().size());
    Histogramer::Fill("EMMA", "IC0.size()", 10, 0, 10, emma->IC0().size());
    Histogramer::Fill("EMMA", "IC1.size()", 10, 0, 10, emma->IC1().size());
    Histogramer::Fill("EMMA", "IC2.size()", 10, 0, 10, emma->IC2().size());
    Histogramer::Fill("EMMA", "IC3.size()", 10, 0, 10, emma->IC3().size());

    if(!emma->Si().empty() && !emma->IC0().empty()) 
      Histogramer::Fill("PID","Si vs IC0", 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC0()[0].Charge());
    if(!emma->Si().empty() && !emma->IC1().empty()) 
      Histogramer::Fill("PID","Si vs IC1", 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC1()[0].Charge());
    if(!emma->Si().empty() && !emma->IC2().empty())
      Histogramer::Fill("PID","Si vs IC2", 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC2()[0].Charge());
    if(!emma->Si().empty() && !emma->IC3().empty()) 
      Histogramer::Fill("PID","Si vs IC3", 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC3()[0].Charge());
  
    for(int i=0;i<tig->Hits().size();i++){
      TigressHit tighit = tig->Hits()[i];
      double e1 = tighit.Doppler(beta);
      long t1   = tighit.TimestampNs();
      if(e1<15) continue;
      if(tighit.KValue()!=379) continue;
      Histogramer::Fill("TIG","single", 8e3,0,8e3,tighit.Energy());
      Histogramer::Fill("TIG",Form("Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      Histogramer::Fill("TIG",Form("summary Doppler(b=%.3f)",beta),64,0,64,tighit.ArrayNumber(),4e3,0,4e3,e1);
      Histogramer::Fill("PGAC", Form("PGACX vs Doppler(b=%.3f)",beta), 60,-30,30,emma->PGACX(), 4e3,0,4e3,e1);
      if(!tighit.BGOFire()) Histogramer::Fill("PGAC/Gate", Form("BGO veto: PGACX vs Doppler(b=%.3f)",beta), 60,-30,30,emma->PGACX(), 4e3,0,4e3,e1);
      if(!emma->Si().empty() && !emma->IC0().empty()) 
        Histogramer::Fill("TIG/Gate",Form("Si && IC0 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!emma->Si().empty() && !emma->IC1().empty()) 
        Histogramer::Fill("TIG/Gate",Form("Si && IC1 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!emma->Si().empty() && !emma->IC2().empty())
        Histogramer::Fill("TIG/Gate",Form("Si && IC2 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!emma->Si().empty() && !emma->IC3().empty()) 
        Histogramer::Fill("TIG/Gate",Form("Si && IC3 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!tighit.BGOFire()) {
        Histogramer::Fill("TIG/Gate",Form("!BGO veto: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
        Histogramer::Fill("TIG/Gate",Form("!BGO veto: summary Doppler(b=%.3f)",beta),64,0,64,tighit.ArrayNumber(),4e3,0,4e3,e1);
      }
      if(emma->PGACX()>-15 && emma->PGACX()<-6){
        if(e1>=1250 && e1<=1300){
          if(!emma->Si().empty() && !emma->IC0().empty()) 
            Histogramer::Fill("PID/Gate",Form("Si vs IC0: Doppler(%.3f)=[1250,1300]keV && pGACX=(-15,-6)",beta), 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC0()[0].Charge());
        }  
        if(e1>=1200 && e1<=1250){
          if(!emma->Si().empty() && !emma->IC0().empty()) 
            Histogramer::Fill("PID/Gate",Form("Si vs IC0: Doppler(%.3f)=[1200,1250]keV && pGACX=(-15,-6)",beta), 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC0()[0].Charge());
        }  
        if(e1>=1330 && e1<=1380){
          if(!emma->Si().empty() && !emma->IC0().empty()) 
            Histogramer::Fill("PID/Gate",Form("Si vs IC0: Doppler(%.3f)=[1330,1380]keV && pGACX=(-15,-6)",beta), 4e3,0,4e3,emma->Si()[0].Charge(), 4e3,0,4e3,emma->IC0()[0].Charge());
        }  
      }
       

      for(int j=i+1;j<tig->Hits().size();j++){
        TigressHit tighit2 = tig->Hits()[j];
        double e2 = tighit2.Doppler(beta);
        long t2   = tighit2.TimestampNs();   
        if(e2<15) continue;
        if(tighit2.KValue()!=379) continue;
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
          if((!tighit.BGOFire()) && (!tighit2.BGOFire())){
            Histogramer::Fill("TIG/Coinc/Gate",Form("gg matrix(BGO veto): Doppler(%.3f) within dtns=[-50,150]ns",beta),4000,0,4000,e1,4000,0,4000,e2);
            Histogramer::Fill("TIG/Coinc/Gate",Form("gg matrix(BGO veto): Doppler(%.3f) within dtns=[-50,150]ns",beta),4000,0,4000,e2,4000,0,4000,e1);
          }
        }
      } // loop j over
    } // loop i over
     
 
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
