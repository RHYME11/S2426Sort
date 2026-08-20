// c++ $(root-config --cflags) -Iinclude macros/cxx/physicstree_promptgood.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lS2426PHYSICS -lTMIDAS -lCHANNEL $(root-config --libs) -o macros/cxx/bin/physicstree_promptgood

#include <cstdio>

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <Channel.h>
#include <Emma.h>
#include <Histogramer.h>
#include <Tigress.h>

// ============== main ==============
// Purpose: Fill histograms from PromptGoodTree.
// Inputs: Physics ROOT filename.
// Outputs: Histogram ROOT file.
int main(int argc, char** argv) {
  if(argc != 2) {
    std::printf("usage: %s physics<run>_<subrun>.root\n", argv[0]);
    return 1;
  }

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");

  TFile* infile = TFile::Open(Form("ttreeOutput/%s", argv[1]));
  if(!infile || infile->IsZombie()) {
    std::printf("failed to open ttreeOutput/%s\n", argv[1]);
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

  Histogramer::Get()->SetOutputPath(
    Form("histOutput/physicstree_promptgood/hist_%s", argv[1]));

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
      if(e1<15) continue;
      if(tighit.KValue()!=379) continue;
      Histogramer::Fill("TIG","single", 8e3,0,8e3,tighit.Energy());
      Histogramer::Fill("TIG",Form("Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      Histogramer::Fill("PGAC", Form("PGACX vs Doppler(b=%.3f)",beta), 60,-30,30,emma->PGACX(), 4e3,0,4e3,e1);
      if(!emma->Si().empty() && !emma->IC0().empty()) 
        Histogramer::Fill("TIG/Gate",Form("Si && IC0 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!emma->Si().empty() && !emma->IC1().empty()) 
        Histogramer::Fill("TIG/Gate",Form("Si && IC1 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!emma->Si().empty() && !emma->IC2().empty())
        Histogramer::Fill("TIG/Gate",Form("Si && IC2 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!emma->Si().empty() && !emma->IC3().empty()) 
        Histogramer::Fill("TIG/Gate",Form("Si && IC3 trigger: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
      if(!tighit.BGOFire()) 
        Histogramer::Fill("TIG/Gate",Form("!BGO veto: Doppler(b=%.3f)",beta),8e3,0,8e3,e1);
    }
     
 
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
