void drawClasses(TH1 *h, const std::string &options)
{
  TCanvas c;
  h->Draw(options.c_str());
  c.SetLogz();
  c.SaveAs(TString::Format("fig/%s.pdf", h->GetName()));

  TH1 *proj = ((TH2*)h)->ProjectionX();
  proj->Draw();
  c.SetLogy();
  c.SaveAs(TString::Format("fig/%s_proj.pdf", h->GetName()));
}

void plot(const std::string &name = "histos.root")
{
  std::unique_ptr<TFile> file(TFile::Open(name.c_str(), "read"));
  std::map<std::string, TH1*> histoMap;

  std::map<std::string, std::string> drawOptions;
  drawOptions["classes"] = "";

  std::map<std::string, std::function<void(TH1*, const std::string &options)> > drawFunctions;
  drawFunctions.emplace("classes", &drawClasses);

  TIter next(file->GetListOfKeys());
  while (TKey *key = (TKey*) next()) {
    TObject *obj = key->ReadObj();
    if (obj && obj->InheritsFrom("TH1"))
      histoMap[obj->GetName()] = (TH1*) obj;
  }

  for (const auto kv : histoMap) {
    std::string options = "";

    if (kv.second->InheritsFrom("TH2"))
      options += "colz";

    if (kv.second->GetXaxis()->IsAlphanumeric()) {
      kv.second->LabelsOption("a", "X");
      kv.second->LabelsDeflate("X");
    }
    if (kv.second->GetYaxis()->IsAlphanumeric()) {
      kv.second->LabelsOption("a", "Y");
      kv.second->LabelsDeflate("Y");
    }

    if (drawFunctions.count(kv.first) != 0) {
      drawFunctions[kv.first](kv.second, options);
      continue;
    }

    TCanvas c;

    if (kv.second->GetDimension() == 1)
      c.SetLogy();

    kv.second->Draw(options.c_str());
    c.SaveAs(TString::Format("fig/%s.pdf", kv.second->GetName()));
  }
}
