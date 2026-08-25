

void Count(){

  TFile *frgfile = TFile::Open("/data1/yzhu/Projects/S2426/S2426Sort_main/ttreeOutput/fragment62261_001.root");
  TFile *evefile = TFile::Open("/data1/yzhu/Projects/S2426/S2426Sort_main/ttreeOutput/event62261_001.root");
  TFile *phyfile = TFile::Open("/data1/yzhu/Projects/S2426/S2426Sort_main/ttreeOutput/physics62261_001.root");
  TTree *FragmentTree = (TTree *)frgfile->Get("FragmentTree");
  TTree *EventTree = (TTree *)evefile->Get("EventTree");
  TTree *PromptGoodTree = (TTree *)phyfile->Get("PromptGoodTree");
  TTree *PromptBadTree = (TTree *)phyfile->Get("PromptBadTree");
  TTree *BgTree = (TTree *)phyfile->Get("BgTree");
  Channel::Read("cal/CalibrationFile_May1526_pol1.cal");
   
  Fragment *ftg = nullptr;
  FragmentTree->SetBranchAddress("Fragment", &ftg);
  DetectorEvent *eve = nullptr;
  EventTree->SetBranchAddress("DetectorEvent", &eve);
  Tigress *tig = nullptr;
  Emma *emma = nullptr;
  PromptGoodTree->SetBranchAddress("Tigress", &tig);
  PromptGoodTree->SetBranchAddress("Emma",    &emma); 

  long count0 = 0; // sum of fragments form event tree
  long count1 = 0; // no anode eventtree entry = bgtree
  long count2 = 0; // eventree: has anode, no left & right = promptbad 
  long count3 = 0; // eventtree: has anode, left || right = promptgood 
  long count4 = 0; // eventtree: has anode, left && right
  long count5 = 0; // eventtree: has anode, left && right, core
  long count6 = 0; // promptgoodtree: has anode, left && right
  long count7 = 0; // promptgoodtree: has anode, left && right, core

  for(long x=0;x<EventTree->GetEntries();x++){
    EventTree->GetEntry(x);
    count0 += eve->Fragments().size();
    bool anode_flag = false;
    bool left_flag = false;
    bool right_flag = false;
    bool core_flag = false;
    for(int i=0;i<eve->Fragments().size();i++){
      int number = eve->Fragments()[i].Number();
      if(number>=866 && number<=868) anode_flag = true;
      if(number == 869) right_flag = true;
      if(number == 870) left_flag = true;
      if(number<720 && ((number%15)==9 || (number%15)==10)) core_flag = true;
    } // i loop over
    if(!anode_flag) count1++;  
    else{
      if((!left_flag) && (!right_flag)) count2++;
      if(left_flag || right_flag) count3++;
      if(left_flag && right_flag) {
        count4++;
        if(core_flag) count5++;
      }
    } 
  }

  for(long x=0;x<PromptGoodTree->GetEntries();x++){
    PromptGoodTree->GetEntry(x);
    if(!emma->Left().empty() && !emma->Right().empty()) {
      count6++;
      if(!tig->Hits().empty()) count7++;
    }
  }
  


  printf("FragmentTree Entries                          = %lld\n", FragmentTree->GetEntries());
  printf("EventTree.fFragments.size()                   = %lu\n\n", count0);
  printf("EventTree Entries                             = %lld\n", EventTree->GetEntries());
  printf("BgTree Entries                                = %lld\n", BgTree->GetEntries());
  printf("PromptGoodTree Entries                        = %lld\n", PromptGoodTree->GetEntries());
  printf("PromptBadTree Entries                         = %lld\n\n", PromptBadTree->GetEntries());
  printf("EventTree: no anode                           = %lu\n", count1);
  printf("EventTree: has anode: no left && right        = %lu\n", count2);
  printf("EventTree: has anode: left || right           = %lu\n", count3);
  printf("EventTree: has anode: left && right           = %lu\n", count4);
  printf("EventTree: has anode: left && right           = %lu\n", count4);
  printf("EventTree: has anode: left && right, has core = %lu\n", count5);
  printf("PromptGoodTree: left && right                 = %lu\n", count6);
  printf("PromptGoodTree: left && right, has core       = %lu\n", count7);

}
