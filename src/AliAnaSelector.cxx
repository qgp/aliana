#include <cstdio>
#include <cstdarg>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "THn.h"
#include "AliVTrack.h"
#include "TCanvas.h"

#include "AliAnaSelector.h"

AliAnaSelector::AliAnaSelector(TTree*) :
  TSelector()
{

}

AliAnaSelector::~AliAnaSelector()
{

}

AliAnaSelector *AliAnaSelector::CreateSelector(const std::string &name)
{
  return (AliAnaSelector*) TClass::GetClass(name.c_str())->New();
}

void AliAnaSelector::Init(TTree *tree)
{
  fReader.SetTree(tree);
}

Bool_t AliAnaSelector::Notify()
{
  return TSelector::Notify();
}

void AliAnaSelector::SlaveBegin(TTree *tree)
{
  UserInit();

  // register histograms
  for (const auto kv : fHistoMap)
    GetOutputList()->Add(kv.second);
}

Bool_t AliAnaSelector::Process(Long64_t entry)
{
  fReader.SetLocalEntry(entry);

  return UserProcess();
}

void AliAnaSelector::SlaveTerminate()
{

}

void AliAnaSelector::Terminate()
{
  // draw histograms

  if (fHistoMap.size() == 0) {
    TIter next(GetOutputList());
    while (TObject* obj = next())
      if (TH1 *h = dynamic_cast<TH1*> (obj)) {
        const std::string name = h->GetName();
        fHistoMap[name] = h;
      }
  }

  std::unique_ptr<TFile> outFile(TFile::Open("histos.root", "recreate"));

  for (const auto kv : fHistoMap)
    outFile->WriteTObject(kv.second);
}

TH1 *AliAnaSelector::AddHistogram(TH1 *h)
{
  const std::string name = h->GetName();
  fHistoMap[name] = h;
  return h;
}

TH1 *AliAnaSelector::GetHistogram(const std::string &name)
{
  return fHistoMap[name];
}
