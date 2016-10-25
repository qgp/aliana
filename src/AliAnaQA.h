#ifndef ALIANAQA_H
#define ALIANAQA_H

#include "TTreeReaderArray.h"

#include "AliAnaSelector.h"

class AliAnaQA : public AliAnaSelector
{
public:
  AliAnaQA(TTree * = 0);
  ~AliAnaQA() {}

  virtual void    UserInit();
  virtual Bool_t  UserProcess();

  Bool_t IsGoodEvent();
  Bool_t IsOutOfBunchPileUp();

protected:
  std::map<Int_t, Int_t> fRunIndex;

  TTreeReaderArray<Int_t> fNofITSClusters;

  ClassDef(AliAnaQA, 0);
};

#endif
