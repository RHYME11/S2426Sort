// c++ $(root-config --cflags) -Iinclude macros/cxx/fragmenttree.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lTMIDAS -lCHANNEL $(root-config --libs) -o macros/cxx/bin/fragmenttree

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <Fragment.h>
#include <Channel.h>
#include <Histogramer.h>
#include <Tigress.h>

namespace {

//============== ParseDetectorNumber ==============
// Purpose: Validate the TIGRESS detector number for a core or BGO channel.
// Inputs: TIG or TIS channel name.
// Outputs: Detector number 5--16, or -1 for an invalid name.
int ParseDetectorNumber(const std::string& name) {
  if(name.size() < 9 ||
     (name.compare(0, 3, "TIG") != 0 && name.compare(0, 3, "TIS") != 0) ||
     name[3] < '0' || name[3] > '9' ||
     name[4] < '0' || name[4] > '9') {
    return -1;
  }
  const int detector = (name[3] - '0') * 10 + (name[4] - '0');
  return detector >= 5 && detector <= 16 ? detector : -1;
}

//============== ParseCrystalNumber ==============
// Purpose: Map a core channel's crystal color to its array offset.
// Inputs: TIGRESS channel name.
// Outputs: Crystal number 0--3, or -1 for an invalid name.
int ParseCrystalNumber(const std::string& name) {
  if(name.size() <= 5) return -1;
  switch(name[5]) {
    case 'B': return 0;
    case 'G': return 1;
    case 'R': return 2;
    case 'W': return 3;
    default: return -1;
  }
}

//============== BGOFire ==============
// Purpose: Find a coincident BGO above the shared TIGRESS charge threshold.
// Inputs: Core Time() in ns and sorted, threshold-selected BGO times.
// Outputs: True if a BGO lies strictly inside the suppression window.
bool BGOFire(double coreTime, const std::vector<double>& bgoTimes) {
  const auto& window = Tigress::SUPPRESSION_WINDOW_NS;
  const double lower = coreTime - window[1];
  const double upper = coreTime - window[0];
  const auto it = std::upper_bound(bgoTimes.begin(), bgoTimes.end(), lower);
  return it != bgoTimes.end() && *it < upper;
}

}

//============== main ==============
// Purpose: Fill CoreA histograms after vetoing coincident BGO triggers.
// Inputs: ROOT file containing FragmentTree/Fragment.
// Outputs: histOutput/fragmenttree/hist_<input basename> for accepted cores.
int main(int argc, char** argv) {
  if(argc != 2) {
    std::printf("usage: %s path/to/fragment<run>_<subrun>.root\n", argv[0]);
    return 1;
  }

  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");

  const std::filesystem::path inputPath(argv[1]);
  const std::string inputName = inputPath.filename().string();
  std::unique_ptr<TFile> infile(TFile::Open(inputPath.c_str()));
  if(!infile || infile->IsZombie()) {
    std::printf("failed to open %s\n", inputPath.c_str());
    return 1;
  }

  TTree* tree = nullptr;
  infile->GetObject("FragmentTree", tree);
  if(!tree) {
    std::printf("FragmentTree not found\n");
    infile->Close();
    return 1;
  }

  Fragment* ftg = nullptr;
  if(tree->SetBranchAddress("Fragment", &ftg) < 0) {
    std::fprintf(stderr, "failed to bind Fragment branch\n");
    return 1;
  }

  // Copy times rather than pointers: GetEntry reuses the branch object.
  std::array<std::vector<double>, 17> bgoTimesByDetector;
  const Long64_t nentries = tree->GetEntries();
  for(Long64_t entry = 0; entry < nentries; ++entry) {
    if(tree->GetEntry(entry) <= 0 || !ftg) {
      std::fprintf(stderr, "failed to read BGO pass entry %lld\n", entry);
      tree->ResetBranchAddresses();
      delete ftg;
      return 1;
    }
    if(entry % 50000 == 0) {
      std::printf("BGO pass: %lld / %lld\r", entry, nentries);
      std::fflush(stdout);
    }
    if(ftg->DetType() != 3 || !(ftg->Charge() > Tigress::SUPPRESSION_CHARGE)) {
      continue;
    }
    const int detector = ParseDetectorNumber(ftg->Name());
    const double time = ftg->Time();
    if(detector < 0 || !std::isfinite(time) || time < 0.) continue;
    bgoTimesByDetector[detector].push_back(time);
  }
  for(auto& times : bgoTimesByDetector) {
    std::sort(times.begin(), times.end());
  }
  std::printf("BGO pass: %lld / %lld\n", nentries, nentries);

  const std::string outputPath =
    "histOutput/fragmenttree/hist_" + inputName;
  Histogramer::Get()->SetOutputPath(outputPath);

  Long64_t accepted = 0;
  Long64_t vetoed = 0;
  Long64_t invalid = 0;
  for(Long64_t xentry = 0; xentry < nentries; ++xentry) {
    if(tree->GetEntry(xentry) <= 0 || !ftg) {
      std::fprintf(stderr, "failed to read CoreA pass entry %lld\n", xentry);
      tree->ResetBranchAddresses();
      delete ftg;
      return 1;
    }
    if(xentry % 50000 == 0) {
      std::printf("CoreA pass: %lld / %lld\r", xentry, nentries);
      std::fflush(stdout);
    }
    if(ftg->DetType() != 0) continue;
    const std::string name = ftg->Name();
    const int detnum = ParseDetectorNumber(name);
    const int xtalnum = ParseCrystalNumber(name);
    const double coreTime = ftg->Time();
    if(detnum < 0 || xtalnum < 0 ||
       !std::isfinite(coreTime) || coreTime < 0.) {
      ++invalid;
      continue;
    }
    if(BGOFire(coreTime, bgoTimesByDetector[detnum])) {
      ++vetoed;
      continue;
    }
    ++accepted;
    const double e = ftg->Energy();
    const double chg = ftg->Charge();
    const long tsns = ftg->TimestampNs();
    const int arrynum = (detnum - 1) * 4 + xtalnum;
    Histogramer::Fill("sum_charge",64,0,64,arrynum, 4e3,0,16e3,chg);
    Histogramer::Fill("sum_energy",64,0,64,arrynum, 4e3,0,4e3 ,e);
    Histogramer::Fill(Form("Array%i",arrynum),"time/10sec_energy", 7000,0,7000,tsns/1.e10, 4e3,0,4e3 ,e);
    Histogramer::Fill(Form("Array%i",arrynum),"energy"     , 4e3,0,4e3 ,e);
  }
  std::printf("CoreA pass: %lld / %lld; accepted=%lld, BGO vetoed=%lld, invalid=%lld\n",
              nentries, nentries, accepted, vetoed, invalid);

  Histogramer::Close();
  tree->ResetBranchAddresses();
  delete ftg;
  infile->Close();

  return 0;
}
