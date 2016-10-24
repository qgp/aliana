#ifndef PLOTTER_H
#define PLOTTER_H

#include "TFile.h"

class TH1;
class TFile;

class plotter
{
public:
  plotter(const std::string &filename = "histos.root");
  ~plotter();

  void register_draw(std::string, std::function<void(TH1*, const std::string&)>);
  void plot();

protected:
  std::unique_ptr<TFile> fFile;
  std::map<std::string, std::string> fDrawOptions;
  std::map<std::string, std::function<void(TH1*, const std::string&)> > fDrawFunctions;
};

#endif
