#ifndef ALITREEREADERVALUE_H
#define ALITREEREADERVALUE_H

#include "TTreeReaderValue.h"

template<class T>
class AliTreeReaderValue : public TTreeReaderValue<T> {
 public:
  using TTreeReaderValue<T>::TTreeReaderValue;

 protected:

  // ClassDef(AliTreeReaderValue, 1);
};

#endif
