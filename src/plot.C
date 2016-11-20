Bool_t drawClasses(TH1 *h, const std::string &options)
{
  TCanvas c;
  h->Draw(options.c_str());
  c.SetLogz();
  c.SaveAs(TString::Format("fig/%s.pdf", h->GetName()));

  TH1 *proj = ((TH2*)h)->ProjectionX();
  proj->Draw();
  c.SetLogy();
  c.SaveAs(TString::Format("fig/%s_proj.pdf", h->GetName()));

  return kFALSE;
}

Bool_t drawEtaPhi(TH1 *h, const std::string &options)
{
  TCanvas c;
  TH1 *proj = ((TH2*)h)->ProjectionX();
  if (proj->GetEntries() > 0)
    proj->Scale(1./proj->GetEntries());
  proj->Draw();
  // c.SetLogy();
  c.SaveAs(TString::Format("fig/%s_eta.pdf", h->GetName()));

  return kTRUE;
}

Bool_t drawPercentile(TH1 *h, const std::string &options)
{
  TCanvas c;
  h->Draw(options.c_str());
  c.SetLogz();
  c.SaveAs(TString::Format("fig/%s.pdf", h->GetName()));

  TH1 *proj = ((TH2*)h)->ProjectionX();
  proj->Draw();
  c.SetLogy();
  c.SaveAs(TString::Format("fig/%s_proj.pdf", h->GetName()));

  return kFALSE;
}

void plot(const std::string &name = "histos.root")
{
  plotter pl(name);
  pl.register_draw("classes", &drawClasses);
  pl.register_draw("etaphi", &drawEtaPhi);
  pl.register_draw("perc_v0m", &drawPercentile);
  pl.register_draw("perc_tkl", &drawPercentile);

  pl.plot();
}
