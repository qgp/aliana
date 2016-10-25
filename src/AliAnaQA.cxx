#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

#include "AliVTrack.h"
#include "AliVEvent.h"

#include "AliAnaQA.h"

AliAnaQA::AliAnaQA(TTree *)
  : AliAnaSelector(),
    fNofITSClusters(fReader, "nofITSClusters.fNofITSClusters")
{
  AddValue<Int_t>("run");
  AddValue<UInt_t>("classes");
  AddValue<Float_t>("multV0Meq");
  AddValue<Float_t>("multmeanV0A");
  AddValue<Float_t>("multmeanV0C");
  AddValue<Float_t>("multmeanV0M");
  AddValue<Float_t>("multmeanTKL");
  AddValue<Float_t>("perc_V0M", "multpercV0Meq");
  AddValue<Float_t>("perc_TKL", "multpercTKL");
  AddValue<Int_t>("V0ADecision");
  AddValue<Int_t>("V0CDecision");
  AddValue<Int_t>("IsIncomplete");
  AddValue<UInt_t>("mask");
  AddValue<TClonesArray>("tracks");
  AddValue<UInt_t>("vtxContributors");
  AddValue<UInt_t>("spdvtxContributors");
  AddValue<Float_t>("vtxz");
  AddValue<Float_t>("spdvtxz");
  AddValue<TBits>("IR1");
  AddValue<UInt_t>("onlineSPD");
  AddValue<UInt_t>("offlineSPD");
  AddValue<Float_t>("multTKL");
  AddValue<Bool_t>("pileupspd");
}

void AliAnaQA::UserInit()
{
  AddHistogram(new TH1F("ntracks", "no. of tracks;N_{trk};counts",
                        200, -.5, 199.5));
  AddHistogram(new TH2F("classes", "classes;class;run",
                        5, -.5, 4.5,
                        1, 0, 0));
  GetHistogram("classes")->GetXaxis()->SetBinLabel(1, "CINT7-B-");
  GetHistogram("classes")->GetXaxis()->SetBinLabel(2, "CMSL7-B-");
  GetHistogram("classes")->GetXaxis()->SetBinLabel(3, "CMSH7-B-");
  GetHistogram("classes")->GetXaxis()->SetBinLabel(4, "CVHMV0M-B-");
  GetHistogram("classes")->GetXaxis()->SetBinLabel(5, "CVHMSH2-B-");
  AddHistogram(new TH3D("stats","statistics;centrality;;",
                        100, 0., 100.,
                        1, 0, 1,
                        1, 0, 1));
  AddHistogram(new TH2F("etaphi", "tracks;#eta;#varphi",
                        200, -1., 1.,
                        200, 0., 2*TMath::Pi()));
  AddHistogram(new TH1F("runs", "run;counts",
                        1, 0, 0));
  AddHistogram(new TH2F("perc_v0m", "V0M percentile;percentile;run;counts",
                        100, 0., 100.,
                        1, 0, 0));
  AddHistogram(new TH2F("perc_tkl", "TKL percentile;percentile;run;counts",
                        50, 0., 100.,
                        1, 0, 0));
  AddHistogram(new TH1F("vtxz", "vertex;z_{vtx} (cm);counts",
                        100, -20., 20.));
  AddHistogram(new TH1F("vtxz_spd", "SPD vertex;z_{vtx}^{SPD} (cm);counts",
                        100, -20., 20.));
}

Bool_t AliAnaQA::UserProcess()
{
  const Int_t run = GetValue<Int_t>("run");
  const TString runName = TString::Format("%i", run);

  const Int_t nClasses = 5;
  const UInt_t classes = GetValue<UInt_t>("classes");
  for (Int_t iClass = 0; iClass < nClasses; ++iClass)
    if ((classes & (1 << iClass)) != 0)
      ((TH2*) GetHistogram("classes"))->Fill(iClass, runName, 1.);

  GetHistogram("runs")->Fill(runName.Data(), 1.);

  // if (fRunIndex.count(run) == 0) {
  //   GetHistogram("runs")->Fill(runName.Data(), 1.);
  //   fRunIndex[run] = GetHistogram("runs")->GetXaxis()->FindBin(runName);
  //   GetHistogram("runs")->GetXaxis()->SetBinLabel(fRunIndex[run], runName);

  //   // ((TH2*) GetHistogram("perc_v0m"))->Fill(-1., runName.Data(), 0.);
  //   // ((TH2*) GetHistogram("perc_tkl"))->Fill(-1., runName.Data(), 0.);
  // }
  // else {
  //   GetHistogram("runs")->Fill(fRunIndex[run]);
  // }

  // if (run != 259261)
  //   return kTRUE;

  if (!IsGoodEvent())
    return kTRUE;

  // ((TH2*) GetHistogram("perc_v0m"))->Fill(GetValue<Float_t>("perc_V0M"), fRunIndex[run] + 1, 1.);
  // ((TH2*) GetHistogram("perc_tkl"))->Fill(GetValue<Float_t>("perc_TKL"), fRunIndex[run] + 1, 1.);
  ((TH2*) GetHistogram("perc_v0m"))->Fill(GetValue<Float_t>("perc_V0M"), runName.Data(), 1.);
  ((TH2*) GetHistogram("perc_tkl"))->Fill(GetValue<Float_t>("perc_TKL"), runName.Data(), 1.);

  GetHistogram("vtxz")->Fill(GetValue<Float_t>("vtxz"));
  GetHistogram("vtxz_spd")->Fill(GetValue<Float_t>("spdvtxz"));

  TClonesArray *tracks = GetPointer<TClonesArray>("tracks");
  const Int_t nTracks = tracks->GetEntriesFast();

  GetHistogram("ntracks")->Fill(nTracks);
  for (Int_t iTrack = 0; iTrack < nTracks; ++iTrack) {
    if (AliVTrack *track = (AliVTrack*) (*tracks)[iTrack])
      GetHistogram("etaphi")->Fill(track->Eta(), track->Phi());
  }

  return kTRUE;
}

Bool_t AliAnaQA::IsGoodEvent()
{
  const Float_t cent = GetValue<Float_t>("perc_V0M");
  const TString runName = TString::Format("%i", GetValue<Int_t>("run"));

  // all
  ((TH3*) GetHistogram("stats"))->Fill(cent, "all", runName.Data(), 1.);

  // physics selection
  if ((GetValue<Int_t>("V0ADecision") != 1) ||
      (GetValue<Int_t>("V0CDecision") != 1))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after V0 decision", runName.Data(), 1.);

  // incomplete events
  if (GetValue<Int_t>("IsIncomplete"))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after incomplete events", runName.Data(), 1.);

  // //V0 asymmetry cut
  // if (IsAsymmetricV0())
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after V0 asymmetry cut",Form("%i",fCurrentRunNumber),1.);

  // trigger mask (using physics selection)
  if (!(GetValue<UInt_t>("mask") & AliVEvent::kAny))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after PS", runName.Data(), 1.);

  //trigger check
  UInt_t classfired = 1 << 0; // MB
  if (classfired && !(GetValue<UInt_t>("classes") & classfired))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after trigger", runName.Data(), 1.);

  //mean=0 means that it is not calibrated
  if ((GetValue<Float_t>("multmeanV0M") <= 0) ||
      (GetValue<Float_t>("multmeanTKL") <=0))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "missing mult calib", runName.Data(), 1.);

  if ((GetValue<UInt_t>("vtxContributors") < 1) ||
      (GetValue<UInt_t>("spdvtxContributors") < 1))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after vtx contrib", runName.Data(), 1.);

  if (TMath::Abs(GetValue<Float_t>("vtxz") - GetValue<Float_t>("spdvtxz")) > 0.5)
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after SPD vs primary check", runName.Data(), 1.);

  const Double_t zmin = -10.;
  const Double_t zmax =  10.;
  if ((GetValue<Float_t>("vtxz") > zmax) ||
      (GetValue<Float_t>("vtxz") < zmin))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after zvtx cut", runName.Data(), 1.);

  // stop here for MC
  // if(isMC)
  //   return kTRUE;

  // out-of-bunch 11 BC
  if (IsOutOfBunchPileUp())
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after out-of-bunch pileup", runName.Data(), 1.);

  // online-offline SPD fastor
  if(GetValue<UInt_t>("onlineSPD") <= -20.589 + 0.73664*GetValue<UInt_t>("offlineSPD"))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after on-offline SPD", runName.Data(), 1.);

  // SPD pileup
  if (GetValue<Bool_t>("pileupspd"))
    return kFALSE;
  ((TH3*) GetHistogram("stats"))->Fill(cent, "after SPD pile-up check", runName.Data(), 1.);

  // //tkl-cluster cut
  // UInt_t multTKL = GetValue<Float_t>("multTKL");
  // if (fNofITSClusters.GetSize() > 0) {
  //   if (fNofITSClusters[0]+fNofITSClusters[1] > 64+4*multTKL)
  //     return kFALSE;
  // }
  // else {
  //   printf("ERROR\n");
  // }
  // ((TH3*) GetHistogram("stats"))->Fill(cent, "after tkl-cluster cut", runName.Data(), 1.);

  return kTRUE;
}

Bool_t AliAnaQA::IsOutOfBunchPileUp()
{
  Bool_t bIsOutOfBunchPileup = kFALSE;

  TBits ir1 = GetValue<TBits>("IR1");
  for (Int_t i=1; i<= 3; i++)
    bIsOutOfBunchPileup |= ir1.TestBitNumber(90-i);
  for (Int_t i=3; i <= 11; i++)
    bIsOutOfBunchPileup |= ir1.TestBitNumber(90+i);

  return bIsOutOfBunchPileup;
}
