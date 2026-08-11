void PrintFrag(long ts){ 
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  TFile *infile = TFile::Open("fragment62140_001.root");
  TTree *FragmentTree = (TTree *)infile->Get("FragmentTree");                                         
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;                                                                                    
  double refdt = 80e3;                                                                                 
  for(x=0;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    long thisTime = ftg->TimestampNs();
    double dt = thisTime -ts;
    if(ftg->Number()==999) continue;
    if(dt<-refdt){
      continue;
    }else if(dt>=-refdt && dt<0){
      printf("%lu: %s(%i)\t%lu\t%.0f\n",x,ftg->Name().c_str(), ftg->DetType(),thisTime, dt);
    }else if(dt==0){
      printf("// ========== entry = %lu (%s) ======== //\n",x,ftg->Name().c_str());
    }else if(dt>0 && dt<refdt){
      printf("%lu: %s(%i)\t%lu\t%.0f\n",x,ftg->Name().c_str(), ftg->DetType(),thisTime, dt);
    }else{
      break;
    }         
  }                                                                                           
}                                                                                                 

void SearchEMMAFrag(long ts){
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  TFile *infile = TFile::Open("fragment62140_001.root");
  TTree *FragmentTree = (TTree *)infile->Get("FragmentTree");                                         
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);
  long nentries = FragmentTree->GetEntries();                                                      
  long x = 0;                                                                                    
  double refdt = 1.5e3;                                                                                 
  for(x=0;x<nentries;x++){                                                                         
    FragmentTree->GetEntry(x);
    if(ftg->Number()<849) continue;
    double dt = ftg->TimestampNs() - ts;
    if(dt==0) printf("// ========== entry = %lu (%s) ======== //\n",x,ftg->Name().c_str());
    if(fabs(dt)<=refdt){
      printf("entry=%lu, dt = %.0f, %s(%04x): t = %lu, c=%.2f\n",x,dt,ftg->Name().c_str(),ftg->Address(),ftg->TimestampNs(), ftg->Charge());
    }
  }
  printf("Loop over!\n");
}
                                                                                                 

void PrintPrompt(long ts){ 
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
  TFile *infile2 = TFile::Open("event62140_001.root");
  TTree *PromptTree = (TTree *)infile2->Get("PromptTree");                                         
  DetectorEvent *eve = nullptr;
  PromptTree->SetBranchAddress("event", &eve);
  long nentries = PromptTree->GetEntries();                                                      
  long x = 0;                                                                                    
  for(x=0;x<nentries;x++){                                                                         
    PromptTree->GetEntry(x);
    long thisTime = eve->timestampNs;
    double dt = thisTime -ts;
    if(dt==0){
      printf("prompt entry = %lu\n",x);
      break;
    }
  }                                                                                              
                                                                                                 
}                                                                                                 

// ======================================================== //
void checkDuplicates(const vector<long>& v, const string& name)
{
    unordered_map<long, int> counts;

    for (long value : v) {
        ++counts[value];
    }

    for (const auto& [value, count] : counts) {
        if (count > 1) {
            cout << name
                 << "  " << value
                 << "  duplicate"
                 << "  count=" << count
                 << '\n';
        }
    }
}

void checkDifference(const vector<long>& v1,
                     const vector<long>& v2,
                     const string& name1,
                     const string& name2)
{
    unordered_set<long> s1(v1.begin(), v1.end());
    unordered_set<long> s2(v2.begin(), v2.end());

    for (long value : s1) {
        if (s2.find(value) == s2.end()) {
            cout << name1
                 << "  " << value
                 << "  only exists in " << name1
                 << '\n';
        }
    }

    for (long value : s2) {
        if (s1.find(value) == s1.end()) {
            cout << name2
                 << "  " << value
                 << "  only exists in " << name2
                 << '\n';
        }
    }
}
// ======================================================== //
void Compare(){

  TFile *infile1 = TFile::Open("fragment62140_001.root");
  TTree *FragmentTree = (TTree *)infile1->Get("FragmentTree");                                         
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);
  long nentries1 = FragmentTree->GetEntries();                                                      

  TFile *infile2 = TFile::Open("event62140_001.root");
  TTree *PromptTree = (TTree *)infile2->Get("PromptTree");                                         
  DetectorEvent *eve = nullptr;
  PromptTree->SetBranchAddress("event", &eve);
  long nentries2 = PromptTree->GetEntries();                                                      

  long x = 0;
  std::vector<long> emt_vec2;
  for(x=0;x<nentries2;x++){
    PromptTree->GetEntry(x);
    emt_vec2.push_back(eve->timestampNs);
  }

  std::vector<long> emt_vec1;
  for(x=0;x<nentries1;x++){
    FragmentTree->GetEntry(x);
    if(ftg->DetType()!=8) continue;
    long thisTime = ftg->TimestampNs();
    emt_vec1.push_back(thisTime);
  }

  checkDuplicates(emt_vec1, "emt_vec1"); 
  checkDuplicates(emt_vec2, "emt_vec2"); 
  checkDifference(emt_vec1, emt_vec2, "emt_vec1", "emt_vec2");

}
