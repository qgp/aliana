#include <cstdio>
#include <cstdarg>

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
  // register histograms
  for (const auto kv : fHistoMap)
    GetOutputList()->Add(kv.second);
}

Bool_t AliAnaSelector::Process(Long64_t entry)
{
  fReader.SetEntry(entry);

  return kTRUE;
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

  for (const auto kv : fHistoMap) {
    TCanvas c;
    std::string options = "";

    if (kv.second->GetDimension() == 1)
      c.SetLogy();

    if (kv.second->InheritsFrom("TH2"))
      options += "colz";

    kv.second->Draw(options.c_str());
    c.SaveAs(TString::Format("fig/%s.pdf", kv.second->GetName()));
  }
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

template<class T>
void AliAnaSelector::AddValue(const std::string &name, const std::string &name_orig)
{
  if (fValueMap.count(name) != 0)
    return;

  printf("AddValue(\"%s\", \"%s\")\n", name.c_str(), name_orig.c_str());
  if (name_orig.length() == 0)
    fValueMap[name] = new TTreeReaderValue<T>(fReader, name.c_str());
  else
    fValueMap[name] = new TTreeReaderValue<T>(fReader, name_orig.c_str());
}

template<class T>
T AliAnaSelector::GetValue(const std::string &name, const std::string &name_orig)
{
  T *ptr = GetPointer<T>(name, name_orig);
  if (!ptr)
    ::Fatal("AliAnaSelector", "got null pointer for TTreeReaderValue");

  return *ptr;
}

template<class T>
T* AliAnaSelector::GetPointer(const std::string &name, const std::string &name_orig)
{
  if (fValueMap.count(name) == 0) {
    printf("need to add value for %s, not yet working ...\n", name.c_str());
    ::Fatal("AliAnaSelector", "...");

    AddValue<T>(name, name_orig);
    // fValueMap[name]->CreateProxy();
  }

  // if (!fValueMap[name]->IsValid())
  //   printf("value for \"%s\" not valid: setup/read status: %i/%i\n",
  //          name.c_str(), fValueMap[name]->GetSetupStatus(), fValueMap[name]->GetReadStatus());

  return ((TTreeReaderValue<T>*) fValueMap[name])->Get();
}
