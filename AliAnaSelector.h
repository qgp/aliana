#ifndef QA_SELECTOR_H
#define QA_SELECTOR_H

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

  ClassDef(AliAnaSelector, 0);
};
#endif
