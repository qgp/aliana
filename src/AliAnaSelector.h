#ifndef QA_SELECTOR_H
#define QA_SELECTOR_H

#include <typeindex>

#include "TSelector.h"
#include "TTreeReader.h"
#include "TList.h"
#include "TH1F.h"

#include "AliTreeReaderValue.h"

class AliAnaSelector : public TSelector {
 public:
  AliAnaSelector(TTree * = 0);
  ~AliAnaSelector();

  static AliAnaSelector *CreateSelector(const std::string &name);

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
  std::map<std::string, Bool_t> fTypeChecked;

  ClassDef(AliAnaSelector, 0);
};

template<class T>
void AliAnaSelector::AddValue(const std::string &name, const std::string &name_orig)
{
  if (fValueMap.count(name) != 0)
    return;

  // printf("AddValue(\"%s\", \"%s\")\n", name.c_str(), name_orig.c_str());
  if (name_orig.length() == 0)
    fValueMap[name] = new AliTreeReaderValue<T>(fReader, name.c_str());
  else
    fValueMap[name] = new AliTreeReaderValue<T>(fReader, name_orig.c_str());

  fTypeMap[name] = typeid(T).hash_code();
}

template<class T>
T AliAnaSelector::GetValue(const std::string &name, const std::string &name_orig)
{
  T *ptr = GetPointer<T>(name, name_orig);
  if (!ptr)
    ::Fatal(__FUNCTION__, "got null pointer for AliTreeReaderValue");

  return *ptr;
}

template<class T>
T* AliAnaSelector::GetPointer(const std::string &name, const std::string &name_orig)
{
  // check if registered
  if (fValueMap.count(name) == 0) {
    ::Fatal(__FUNCTION__, "need to add value for %s, not yet working ...", name.c_str());

    AddValue<T>(name, name_orig);
    // fValueMap[name]->CreateProxy();
  }

  // check if requested type consistent with registered
  if (fTypeMap[name] != typeid(T).hash_code()) {
    ::Error("GetPointer", "Inconsistent type requested for %s", name.c_str());
    return 0x0;
  }

  // check if requested type consistent with the branch (only on first reading)
  if ((fTypeChecked.count(name) == 0) ||
      !fTypeChecked[name]) {
    TBranch *br = fReader.GetTree()->GetBranch(fValueMap[name]->GetBranchName());
    TClass *cl;
    EDataType type;
    if (br->GetExpectedType(cl, type) != 0) {
      ::Error(__FUNCTION__, "could not obtain type for branch <%s>", name.c_str());
    }
    else if (type > 0) {
      if (TDataType::GetType(typeid(T)) != type)
        ::Error(__FUNCTION__, "type mismatch for branch <%s>", name.c_str());
    }
    else if (cl) {
      T obj;
      TObject *test = (TObject*) cl->New();
      if (typeid(*test).hash_code() != typeid(obj).hash_code())
        ::Error(__FUNCTION__, "type mismatch for branch <%s>", name.c_str());
    }
    else {
      ::Error(__FUNCTION__, "could not check type for branch <%s>", name.c_str());
    }
    fTypeChecked[name] = kTRUE;
  }

  T *p = ((AliTreeReaderValue<T>*) fValueMap[name])->Get();

  // check if value is valid
  if (!fValueMap[name]->IsValid())
    printf("value for \"%s\" not valid: setup/read status: %i/%i\n",
           name.c_str(), fValueMap[name]->GetSetupStatus(), fValueMap[name]->GetReadStatus());

  return p;
}
#endif
