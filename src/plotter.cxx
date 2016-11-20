#include <string>
#include <memory>
#include <map>

#include "TKey.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"

#include "plotter.h"

plotter::plotter(const std::string &name) :
  fFile(TFile::Open(name.c_str()))
{

}

plotter::~plotter()
{

}

void plotter::register_draw(std::string name, std::function<Bool_t(TH1*, const std::string&)> func)
{
  fDrawFunctions.emplace(name, func);
}

void plotter::plot()
{
  std::map<std::string, TH1*> histoMap;

  TIter next(fFile->GetListOfKeys());
  while (TKey *key = (TKey*) next()) {
    TObject *obj = key->ReadObj();
    if (obj && obj->InheritsFrom("TH1"))
      histoMap[obj->GetName()] = (TH1*) obj;
  }

  for (const auto kv : histoMap) {
    std::string options = fDrawOptions.count(kv.first) != 0 ? fDrawOptions[kv.first] : "";

    if (kv.second->InheritsFrom("TH2"))
      options += "colz";

    if (kv.second->GetXaxis()->IsAlphanumeric()) {
      kv.second->LabelsOption("a", "X");
      kv.second->LabelsDeflate("X");
    }
    if (kv.second->GetYaxis()->IsAlphanumeric()) {
      if (kv.first != "stats")
        kv.second->LabelsOption("a", "Y");
      kv.second->LabelsDeflate("Y");
    }
    if (kv.second->GetZaxis()->IsAlphanumeric()) {
      kv.second->LabelsOption("a", "Z");
      kv.second->LabelsDeflate("Z");
    }

    if (fDrawFunctions.count(kv.first) != 0) {
      if (!fDrawFunctions[kv.first](kv.second, options))
        continue;
    }

    TCanvas c;

    if (kv.second->GetDimension() == 1)
      c.SetLogy();

    kv.second->Draw(options.c_str());
    c.SaveAs(TString::Format("fig/%s.pdf", kv.second->GetName()));
  }
}
