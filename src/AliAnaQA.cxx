#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

#include "AliVTrack.h"

#include "AliAnaQA.h"

AliAnaQA::AliAnaQA(TTree *)
  : AliAnaSelector()
{
  AddValue<Int_t>("run");
  AddValue<UInt_t>("classes");
  AddValue<Float_t>("multV0Meq");
  AddValue<Float_t>("multmeanV0M");
  AddValue<Float_t>("perc_V0M", "multpercV0Meq");
  AddValue<Float_t>("perc_TKL", "multpercTKL");
  AddValue<TClonesArray>("tracks");
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
  AddHistogram(new TH2F("etaphi", "tracks;#eta;#varphi",
                        200, -1., 1.,
                        200, 0., 2*TMath::Pi()));
  AddHistogram(new TH1F("runs", "run;counts",
                        1, 0, 0));
  AddHistogram(new TH2F("perc_v0m", "V0M percentile;V0M percentile;run;counts",
                        100, 0., 100.,
                        1, 0, 0));
  AddHistogram(new TH2F("perc_tkl", "TKL percentile;TKL percentile;run;counts",
                        100, 0., 100.,
                        1, 0, 0));
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

  // pass only for min. bias
  if ((classes & 0x1) == 0)
    return kTRUE;

  // ((TH2*) GetHistogram("perc_v0m"))->Fill(GetValue<Float_t>("perc_V0M"), fRunIndex[run] + 1, 1.);
  // ((TH2*) GetHistogram("perc_tkl"))->Fill(GetValue<Float_t>("perc_TKL"), fRunIndex[run] + 1, 1.);
  ((TH2*) GetHistogram("perc_v0m"))->Fill(GetValue<Float_t>("perc_V0M"), runName.Data(), 1.);
  ((TH2*) GetHistogram("perc_tkl"))->Fill(GetValue<Float_t>("perc_TKL"), runName.Data(), 1.);

  TClonesArray *tracks = GetPointer<TClonesArray>("tracks");
  const Int_t nTracks = tracks->GetEntriesFast();

  GetHistogram("ntracks")->Fill(nTracks);
  for (Int_t iTrack = 0; iTrack < nTracks; ++iTrack) {
    if (AliVTrack *track = (AliVTrack*) (*tracks)[iTrack])
      GetHistogram("etaphi")->Fill(track->Eta(), track->Phi());
  }

  return kTRUE;
}

Bool_t AliAnaQA::IsGoodEvent() const
{
  // //all
  // if(hEventCount)
  //   hEventCount->Fill(cent,"all",Form("%i",fCurrentRunNumber),1.);

  // //bad runs
  // if(fCurrentRunNumber==225611 || fCurrentRunNumber==225609 || fCurrentRunNumber==225589)
  //   return kFALSE; // skip runs with long tails in V0C
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after rejecting bad runs",Form("%i",fCurrentRunNumber),1.);

  // //physics selection
  // if (fV0CDecision!=1 || fV0ADecision!=1)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after V0 decision check",Form("%i",fCurrentRunNumber),1.);

  // //incomplete events
  // if (fIsIncomplete)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after incomplete events",Form("%i",fCurrentRunNumber),1.);

  // //V0 asymmetry cut
  // if (IsAsymmetricV0())
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after V0 asymmetry cut",Form("%i",fCurrentRunNumber),1.);

  // //trigger mask (using physics selection)
  // //  if (!(fSelectMask & triggerMask)) return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after PS check",Form("%i",fCurrentRunNumber),1.);

  // //trigger check
  // if (classfired && !(fClassesFired & classfired))
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after trigger check",Form("%i",fCurrentRunNumber),1.);

  // //mean=0 means that it is not calibrated
  // if (fMultMeanV0M <=0 || fMultMeanTKL <=0)
  //   return kFALSE;
  // if (hEventCount)
  //   hEventCount->Fill(cent,"Missing AliMult calibration",Form("%i",fCurrentRunNumber),1.);

  // // cut events with zvtx==0
  // //if(TMath::Abs(fVtxZ)<1.e-9) cout << fVtxZ << "   " << fNchTPC << endl;
  // //if (TMath::Abs(fVtxZ)<1.e-9) return 0;
  // //if(hEventCount) hEventCount->Fill(cent,"after zvtx=0 cut",Form("%i",fCurrentRunNumber),1.);
  // //  if(!fIsEventSel) return 0; // implement same event selection as in AliMultSelection class (INEL > 0)
  // //  if(hEventCount) hEventCount->Fill(cent,"INEL>0 cut",Form("%i",fCurrentRunNumber),1.);
  // //Z_vtx
  // //SPD vtx contributors
  // //  if (fSPDVtxContributors < 1) return kFALSE;
  // //  if(hEventCount)hEventCount->Fill(cent,"after SPD vtx contributors cut",Form("%i",fCurrentRunNumber),1.);
  // if (fVtxContributors < 1 || fSPDVtxContributors < 1)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after cut on contributors",Form("%i",fCurrentRunNumber),1.);
  // if (TMath::Abs(fVtxZ-fSPDVtxZ)>0.5)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after SPD - primary vtx consistency",Form("%i",fCurrentRunNumber),1.);
  // if (fVtxZ>zmax || fVtxZ<zmin)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after zvtx cut",Form("%i",fCurrentRunNumber),1.);

  // if(isMC)
  //   return kTRUE;

  // //out-of-bunch 11 BC
  // if (IsOutOfBunchPileup())
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after out-of-bunch pileup check",Form("%i",fCurrentRunNumber),1.);
  // //online-offline SPD fastor
  // if(fonlineSPD <= -20.589 + 0.73664*fofflineSPD)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after online-offline SPD fastOR cut",Form("%i",fCurrentRunNumber),1.);
  // //SPD pileup
  // if (fIsPileupSPD)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after SPD pileup check",Form("%i",fCurrentRunNumber),1.);
  // //tkl-cluster cut
  // if (fNofITSClusters[0]+fNofITSClusters[1]>64+4*fMultTKL)
  //   return kFALSE;
  // if(hEventCount)
  //   hEventCount->Fill(cent,"after tkl-cluster cut",Form("%i",fCurrentRunNumber),1.);

  return kTRUE;
}
