// c++ $(root-config --cflags) -Iinclude macros/cxx/fragmenttree.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lTMIDAS -lCHANNEL $(root-config --libs) -o macros/cxx/bin/fragmenttree

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
#include <numeric>
#include <utility>
       
#include <TFile.h>
#include <TTree.h>
#include <Fragment.h>
#include <Channel.h>
#include <Histogramer.h>


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
         
  TTree* tree = (TTree*)infile->Get("FragmentTree");
  if(!tree) {
    std::printf("FragmentTree not found\n");
    infile->Close();
    return 1;
  }

  Fragment *ftg = nullptr;
  tree->SetBranchAddress("Fragment", &ftg);
  const std::string outputPath =      
    "histOutput/fragmenttree/hist_" + inputName;
  Histogramer::Get()->SetOutputPath(outputPath);

  long nentries = tree->GetEntries();
  long xentry = 0;
  for(xentry = 0; xentry < nentries; xentry++) {
    tree->GetEntry(xentry);
    if(ftg->DetType()!=0) continue;
    int    num  = ftg->Number();
    double e    = ftg->Energy();
    double chg  = ftg->Charge(); 
    long   tsns = ftg->TimestampNs();
    std::string name = ftg->Name(); 
    int detnum  = (name[3] - '0') * 10 + (name[4] - '0');
    int xtalnum = [&]() {
                          switch (name[5]) {
                            case 'B': return 0;
                            case 'G': return 1;
                            case 'R': return 2;
                            case 'W': return 3;
                            default:  return -1;
                          }
                        }(); 
    int arrynum = (detnum - 1) * 4 + xtalnum;
    Histogramer::Fill("sum_charge",64,0,64,arrynum, 4e3,0,16e3,chg);
    Histogramer::Fill("sum_energy",64,0,64,arrynum, 4e3,0,4e3 ,e);
    Histogramer::Fill(Form("Array%i",arrynum),"time/10sec_energy", 7000,0,7000,tsns/pow(10,9)/10., 4e3,0,4e3 ,e);
    Histogramer::Fill(Form("Array%i",arrynum),"energy"     , 4e3,0,4e3 ,e);

    if((xentry%5000)==0){
      printf("on entry = %lu / %lu \r", xentry, nentries);
      fflush(stdout);
    }

  }
  printf("on entry = %lu / %lu \n", xentry, nentries);
  
  Histogramer::Close();
  infile->Close();
  delete infile;
      
  return 0;


}









