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

void drawPercentile(TH1 *h, const std::string &options)
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
  plotter pl(name);
  pl.register_draw("classes", &drawClasses);
  pl.register_draw("perc_v0m", &drawPercentile);
  pl.register_draw("perc_tkl", &drawPercentile);

  pl.plot();
}
