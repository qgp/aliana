#ifndef ALIANAQA_H
#define ALIANAQA_H

#include "AliAnaSelector.h"

class AliAnaQA : public AliAnaSelector
{
public:
  AliAnaQA(TTree * = 0);
  ~AliAnaQA() {}

  virtual void    UserInit();
  virtual Bool_t  UserProcess();

  Bool_t IsGoodEvent() const;

protected:
  std::map<Int_t, Int_t> fRunIndex;

  ClassDef(AliAnaQA, 0);
};

#endif
