#ifndef QA_SELECTOR_H
#define QA_SELECTOR_H

#include <typeindex>

#include "TSelector.h"
#include "TTreeReader.h"
#include "TList.h"
#include "TH1F.h"

class AliAnaSelector : public TSelector {
 public:
  AliAnaSelector(TTree * = 0);
  ~AliAnaSelector();

  virtual void    Init(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual void    Terminate();
  virtual void    SlaveTerminate();
  virtual Int_t   Version() const { return 2; }

  virtual void    UserInit() {}
  virtual Bool_t  UserProcess() { return kTRUE; }

  TH1 *AddHistogram(TH1 *h);
  TH1 *GetHistogram(const std::string &name);

  template<class T>
  void AddValue(const std::string &name, const std::string &name_orig = "");
  template<class T>
  T GetValue(const std::string &name, const std::string &name_orig = "");
  template<class T>
  T* GetPointer(const std::string &name, const std::string &name_orig = "");

protected:
  TTreeReader fReader;

  std::map<std::string, TH1*> fHistoMap;
  std::map<std::string, ROOT::Internal::TTreeReaderValueBase*> fValueMap;
  std::map<std::string, size_t> fTypeMap;

  ClassDef(AliAnaSelector, 0);
};

template<class T>
void AliAnaSelector::AddValue(const std::string &name, const std::string &name_orig)
{
  if (fValueMap.count(name) != 0)
    return;

  // printf("AddValue(\"%s\", \"%s\")\n", name.c_str(), name_orig.c_str());
  if (name_orig.length() == 0)
    fValueMap[name] = new TTreeReaderValue<T>(fReader, name.c_str());
  else
    fValueMap[name] = new TTreeReaderValue<T>(fReader, name_orig.c_str());

  fTypeMap[name] = typeid(T).hash_code();
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

  if (fTypeMap[name] != typeid(T).hash_code()) {
    ::Error("AliAnaSelector", "Inconsistent type requested for %s", name.c_str());
    return 0x0;
  }

  // checking for proper type
  const std::string branchname = fValueMap[name]->GetBranchName();
  TBranch *br = fReader.GetTree()->GetBranch(branchname.c_str());
  TClass *cl;
  EDataType type;
  Int_t result = br ? br->GetExpectedType(cl, type) : -1;
  if (result != 0)
    printf("no result for %s\n", name.c_str());

  if (cl) {
    T obj;
    TObject *test = (TObject*) cl->New();
    if (typeid(*test).hash_code() != typeid(obj).hash_code())
      printf("problem\n");
  }
  else if (type > 0) {
    if (TDataType::GetType(typeid(T)) != type)
      printf("problem\n");
  }
  else {
    printf("problem\n");
  }

  // if (!fValueMap[name]->IsValid())
  //   printf("value for \"%s\" not valid: setup/read status: %i/%i\n",
  //          name.c_str(), fValueMap[name]->GetSetupStatus(), fValueMap[name]->GetReadStatus());

  return ((TTreeReaderValue<T>*) fValueMap[name])->Get();
}
#endif
