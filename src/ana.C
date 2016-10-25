#include "AliAnaQA.h"

void ana(const std::string listname = "filelist.txt",
         Bool_t proof = kTRUE,
         Long64_t nEvents = TTree::kMaxEntries)
{
  const std::string ana = "AliAnaQA";

  // prepare chain
  TChain chain("events");
  ifstream infile(listname);
  std::string filename;
  while (infile >> filename)
    chain.Add((filename + "/CorrelationTree/events").c_str());

  // prepare PROOF (if enabled)
  if (proof) {
    TProof::Open("workers=8");

    if (!gProof) {
      ::Error(__FILE__, "Could not open proof connection");
      return;
    }

    gProof->AddDynamicPath(gSystem->GetWorkingDirectory().c_str());
    chain.SetProof();
  }

  // run analysis
  TSelector *selector = (TSelector*) TClass::GetClass(ana.c_str())->New();
  chain.Process(selector, "", nEvents);

  // terminate PROOF (if enabled)
  if (proof) {
    TProofMgr* mgr = gProof->GetManager();
    TProofLog *log = mgr->GetSessionLogs();
    int flag = log->Save("*", "all_workers.log");
  }
}
