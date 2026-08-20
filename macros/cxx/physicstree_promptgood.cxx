// c++ $(root-config --cflags) -Iinclude macros/cxx/physicstree_promptgood.cxx -Lbuild/lib -Wl,-rpath,$PWD/build/lib -lHISTOGRAMER -lPHYSICS -lTMIDAS -lCHANNEL $(root-config --libs) -o physicstree_promptgood

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>

#include <TFile.h>
#include <TTree.h>

#include <Emma.h>
#include <Histogramer.h>

// ============== ResolveInputPath ==============
// Purpose: Use an explicit input path or find a bare filename in ttreeOutput.
// Inputs: User-supplied ROOT filename or path.
// Outputs: Resolved input path.
std::filesystem::path ResolveInputPath(const std::filesystem::path& argument) {
  if(std::filesystem::exists(argument)) {
    return argument;
  }
  return std::filesystem::path("ttreeOutput") / argument;
}

// ============== main ==============
// Purpose: Sort PromptGoodTree entries and write EMMA multiplicity histograms.
// Inputs: One Physics ROOT filename or path.
// Outputs: A ROOT file under histOutput/physicstree_promptgood.
int main(int argc, char** argv) {
  if(argc != 2) {
    std::fprintf(stderr, "usage: %s physics<run>_<subrun>.root\n", argv[0]);
    return 2;
  }

  const std::filesystem::path inputPath = ResolveInputPath(argv[1]);
  if(!std::filesystem::is_regular_file(inputPath)) {
    std::fprintf(stderr, "input file not found: %s\n", inputPath.c_str());
    return 1;
  }

  std::unique_ptr<TFile> inputFile(TFile::Open(inputPath.c_str(), "READ"));
  if(!inputFile || inputFile->IsZombie()) {
    std::fprintf(stderr, "failed to open input file: %s\n", inputPath.c_str());
    return 1;
  }

  TTree* tree = nullptr;
  inputFile->GetObject("PromptGoodTree", tree);
  if(!tree) {
    std::fprintf(stderr, "PromptGoodTree not found in: %s\n", inputPath.c_str());
    return 1;
  }
  if(!tree->GetBranch("Emma")) {
    std::fprintf(stderr, "Emma branch not found in PromptGoodTree\n");
    return 1;
  }

  Emma* emma = nullptr;
  tree->SetBranchStatus("*", false);
  tree->SetBranchStatus("Emma", true);
  if(tree->SetBranchAddress("Emma", &emma) < 0) {
    std::fprintf(stderr, "failed to bind the Emma branch\n");
    return 1;
  }

  const std::filesystem::path outputPath =
    std::filesystem::path("histOutput") / "physicstree_promptgood" /
    (std::string("hist_") + inputPath.filename().string());
  Histogramer::Get()->SetOutputPath(outputPath.string());

  const Long64_t entryCount = tree->GetEntries();
  Long64_t selectedCount = 0;
  bool readFailed = false;
  for(Long64_t entry = 0; entry < entryCount; ++entry) {
    if(tree->GetEntry(entry) < 0) {
      std::fprintf(stderr, "failed to read PromptGoodTree entry %lld\n", entry);
      readFailed = true;
      break;
    }
    if(!emma || emma->Left().empty() || emma->Right().empty()) {
      continue;
    }

    Histogramer::Fill("EMMA", "Si.size()", 10, 0, 10, emma->Si().size());
    Histogramer::Fill("EMMA", "IC0.size()", 10, 0, 10, emma->IC0().size());
    Histogramer::Fill("EMMA", "IC1.size()", 10, 0, 10, emma->IC1().size());
    Histogramer::Fill("EMMA", "IC2.size()", 10, 0, 10, emma->IC2().size());
    Histogramer::Fill("EMMA", "IC3.size()", 10, 0, 10, emma->IC3().size());
    ++selectedCount;
  }

  Histogramer::Close();
  tree->ResetBranchAddresses();
  delete emma;
  inputFile->Close();

  if(readFailed) {
    std::error_code removeError;
    std::filesystem::remove(outputPath, removeError);
    return 1;
  }

  std::printf("sorted %lld PromptGoodTree entries (%lld with Left and Right hits)\n",
              entryCount, selectedCount);
  std::printf("wrote %s\n", outputPath.c_str());
  return 0;
}
