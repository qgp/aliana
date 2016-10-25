#include "TChain.h"
#include "TClonesArray.h"
#include "AliCFParticle.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "AliVEvent.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TLine.h"
#include "TProfile.h"
#include "TF1.h"
#include "TSpline.h"
//#include "TSpline3.h"
#include <iostream>
#include <map>
using namespace std;

#include "../common/utils.h"
#include "../common/env.h"

void drawEventQA(TString filename = "eventQA.root", Int_t itrigger = 0);

void eventQA(
    //char * period="lhc10de",
    //Int_t energy = k7TeV,
    //Int_t anaType = kTklTkl, // this allows us to select the same vtx_z range as in the analysis)
    //Bool_t localinputfiles=kTRUE,
    //    char * pathinputlocalfiles="~/data/trees_pp"
    char * period="lhc15hijl_muon_calo_pass1",
    Int_t energy = k13TeV,
    Int_t anaType = kTrkTrkITS, // this allows us to select the same vtx_z range as in the analysis
    Bool_t localinputfiles=kTRUE,
    //    char * pathinputlocalfiles="~/lbnl5core_data/leonardo/trees_pp" // lhc10d/e/f6a
    char * pathinputlocalfiles="~/data/trees_pp" // lhc10d/e/f6a
        //    char * period="lhc15g3c",
        //    Int_t energy = k13TeV,
        //    Int_t anaType = kTklTklMC, // this allows us to select the same vtx_z range as in the analysis
        //    Bool_t localinputfiles=kTRUE,
        //    char * pathinputlocalfiles="~/lbnl5core_data/leonardo/trees_pp"
        //    char * pathinputlocalfiles="~/data/trees_pp"
) {

  gStyle->SetOptStat(111111);

  TChain* fTree = new TChain("CorrelationTree/events");
  //fTree->AddFile("AnalysisResults_test.root");
  ChainFiles(fTree,localinputfiles,period,pathinputlocalfiles);
  SetTree(fTree);

  // this code snippet automatically figures out how many runs we will process (maxRuns)...
  TCanvas *cTemp = new TCanvas();
  fTree->Draw("run>>hTempCountRuns");
  TH1F *hTempCountRuns = (TH1F*)gDirectory->Get("hTempCountRuns");
  Int_t minRun = Int_t(hTempCountRuns->GetXaxis()->GetBinLowEdge(1));
  Int_t maxRun = Int_t(hTempCountRuns->GetXaxis()->GetBinUpEdge(hTempCountRuns->GetXaxis()->GetNbins()));
  Int_t runBins = maxRun-minRun+1;
  cout << minRun << "   " << maxRun << "   " << runBins << endl;
  TH1F* hTempCountRuns2 = new TH1F("hTempCountRuns2","hTempCountRun2",runBins,minRun,maxRun);
  fTree->Draw("run>>+hTempCountRuns2");
  std::map<Int_t,Int_t> runindex;
  Int_t maxRuns = 0;
  for(Int_t i = 1; i <= hTempCountRuns2->GetXaxis()->GetNbins(); i++){
    if(hTempCountRuns2->GetBinContent(i) > 0) {
      maxRuns++;
      runindex[minRun+i-1] = maxRuns;
      cout << minRun+i-1 << "   " << runindex[minRun+i-1] << endl;
    }
  }
  cout <<"maxRuns:" << maxRuns << endl;

  Double_t zmin = -10., zmax = 10.;
  switch (anaType){
  case kTrkTrk: zmin = xtmin_trktrk[kTrackZvtx]; zmax = xtmax_trktrk[kTrackZvtx]; break;
  case kTrkTrkGen: zmin = xtmin_trktrk[kTrackZvtx]; zmax = xtmax_trktrk[kTrackZvtx]; break;
  case kTklTkl: zmin = xtmin_tkltkl[kTrackZvtx]; zmax = xtmax_tkltkl[kTrackZvtx]; break;
  case kTklTklMC: zmin = xtmin_tkltkl[kTrackZvtx]; zmax = xtmax_tkltkl[kTrackZvtx]; break;
  case kMuTkl: zmin = xtmin_mutkl[kTrackZvtx]; zmax = xtmax_mutkl[kTrackZvtx]; break;
  default: cout << "Invalid z-vertex choice!" << endl;
  }
  cout << "z-vertex range is " << zmin << " - " << zmax << endl;

  //  TFile* multFile = new TFile(Form("../%s/meanmultiplicity.root",period));

  Double_t countRuns = 0;
  Int_t fPrevRun = 0;

  Int_t nEvents = fTree->GetEntries();

  // from Leonardo's QA macro
  Int_t maxTrigs = kNTriggers+1;
  if(energy==k7TeV) // do only MB analysis
    maxTrigs = 1;

  //trigger statistics
  TH2D* hTriggerStat_V0M;
  TH2D* hTriggerStat_TKL;
  hTriggerStat_V0M = new TH2D(Form("hTriggerStat_V0M"),"",maxTrigs,-0.5,maxTrigs-0.5,80,0,800);
  hTriggerStat_TKL = new TH2D(Form("hTriggerStat_TKL"),"",maxTrigs,-0.5,maxTrigs-0.5,80,0,800);
  hTriggerStat_V0M->GetYaxis()->SetTitle("V0M multiplicity");
  hTriggerStat_TKL->GetYaxis()->SetTitle("TKL multiplicity");
  for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++){
    TString trigName;
    if(itrigger == 0) trigName = "kMB";
    else trigName = TriggerName[itrigger-1];
    hTriggerStat_V0M->GetXaxis()->SetBinLabel(itrigger+1,trigName);
    hTriggerStat_TKL->GetXaxis()->SetBinLabel(itrigger+1,trigName);
  }

  TH3D* hEventCount =           new TH3D(          "hEventCount",";centrality;;",100,0.0,10.,1,0,1,maxRuns,0,1);
  TH3D* hEventCount_CVHMV0M_B = new TH3D("hEventCount_CVHMV0M_B",";centrality;;",100,0.0,10.,1,0,1,maxRuns,0,1);

  //create histos
  TH1D* hAllEventsV0M[maxTrigs];
  TH1D* hAllEventsRun[maxTrigs];
  // correlations between percentiles
  TH2F* hV0MTKL[maxTrigs];
  TH2F* hV0AV0CMult[maxTrigs];
  // multiplicities per run
  TH1F* hEventsRun[maxTrigs];
  TH2F* hV0MMultRun[maxTrigs];
  TH2F* hTKLMultRun[maxTrigs];
  // percentiles per run
  TH2F* hV0MCentRun[maxTrigs];
  TH2F* hTKLCentRun[maxTrigs];
  // vertex QA
  TH2F* hVzRun[maxTrigs];
  TH2F* hVzV0MCent[maxTrigs];
  TH2F* hVzTKLCent[maxTrigs];
  // check distributions of V0M and TKL estimators
  TH1D* hNeventsV0M[maxTrigs];
  TH1D* hNeventsTKL[maxTrigs];
  // check multiplicity reach
  TH2D* hV0MNch[maxTrigs];
  TH2D* hTKLNch[maxTrigs];
  TH2F* hV0MCentPerc[maxTrigs];
  TH2F* hV0MCentPercLog[maxTrigs];
  // check SPD vertex vs primary vertex
  TH2D* hVertexSPDprim[maxTrigs];
  TH2D* hVertexSPDprimMult[maxTrigs];
  TH2D* hVertexResMult[maxTrigs];
  TH2D* hVertexContribMult[maxTrigs];
  // check mean multiplicity calibration
  TH2D* hUncorr_V0M[maxTrigs];
  TH2D* hCorr_V0M[maxTrigs];
  TH2D* hUncorr_TKL[maxTrigs];
  TH2D* hCorr_TKL[maxTrigs];
  TH2D* hVzNch[maxTrigs];
  // pileup checks
  TH1D* hAllEvents_V0M[maxTrigs];
  TH1D* hAllEvents_TKL[maxTrigs];
  TH3D* hAllEvents_TklMult_dphi_V0M[maxTrigs];
  TH3D* hAllEvents_TklMult_dphi_TKL[maxTrigs];
  TH3D* hSingleDiffractive_TklMult_dphi_V0M[maxTrigs];
  TH3D* hSingleDiffractive_TklMult_dphi_TKL[maxTrigs];
  TH3D* hDoubleDiffractive_TklMult_dphi_V0M[maxTrigs];
  TH3D* hDoubleDiffractive_TklMult_dphi_TKL[maxTrigs];
  TH3D* hAllEvents_TrkMult_pt_V0M[maxTrigs];
  TH3D* hAllEvents_TrkMult_pt_TKL[maxTrigs];
  TH3D* hSingleDiffractive_TrkMult_pt_V0M[maxTrigs];
  TH3D* hSingleDiffractive_TrkMult_pt_TKL[maxTrigs];
  TH3D* hDoubleDiffractive_TrkMult_pt_V0M[maxTrigs];
  TH3D* hDoubleDiffractive_TrkMult_pt_TKL[maxTrigs];
  TH1D* hPileupSPD_V0M[maxTrigs];
  TH1D* hPileupSPD_TKL[maxTrigs];
  TH1D* hPileupMV_V0M[maxTrigs];
  TH1D* hPileupMV_TKL[maxTrigs];
  TH1D* hTrackClusterCut_V0M[maxTrigs];
  TH1D* hTrackClusterCut_TKL[maxTrigs];
  TH1D* hOutOfBunchPileup_V0M[maxTrigs];
  TH1D* hOutOfBunchPileup_TKL[maxTrigs];
  TH1D* hV0Asymmetry_V0M[maxTrigs];
  TH1D* hV0Asymmetry_TKL[maxTrigs];
  TH1D* hV0Decision_V0M[maxTrigs];
  TH1D* hV0Decision_TKL[maxTrigs];
  TH1D* hIncompleteEvents_V0M[maxTrigs];
  TH1D* hIncompleteEvents_TKL[maxTrigs];
  TH2D* hTrackClusterCorr[maxTrigs];
  TH2D* hTrackClusterCorrCut[maxTrigs];
  TH2D* hOnlineOfflineFastOR[maxTrigs];
  TH2D* hOnlineOfflineFastORCut[maxTrigs];
  TH2D* hV0AsymmCorr[maxTrigs];
  TH2D* hV0AsymmCorrCut[maxTrigs];
  TH2F* h_V0M_TKL[maxTrigs];
  TH2F* hVz_V0M[maxTrigs];
  TH2F* hVz_TKL[maxTrigs];

  for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++){
    TString trigName;
    if(itrigger == 0) trigName = "kMB";
    else trigName = TriggerName[itrigger-1];

    hAllEventsV0M[itrigger] = new TH1D(Form("hAllEventsV0M_%s",trigName.Data()),"",80,0,800); // equalized V0M response
    hAllEventsRun[itrigger] = new TH1D(Form("hAllEventsRun_%s",trigName.Data()),"",maxRuns,0.5,maxRuns+0.5); // equalized V0M response per run
    // correlations between percentiles
    hV0MTKL[itrigger] = new TH2F(Form("hV0MTKL_%s",trigName.Data()),Form("%s;V0M M/<M>;TKL M/<M>",trigName.Data()),50,0,10,50,0,10);
    hV0AV0CMult[itrigger] = new TH2F(Form("hV0AV0CMult_%s",trigName.Data()),Form("%s;V0A multiplicity;V0C multiplicity",trigName.Data()),50,0,300,50,0,600);
    // multiplicities per run
    hEventsRun[itrigger] = new TH1F(Form("hEventsRun_%s",trigName.Data()),Form("%s Events per run",trigName.Data()),maxRuns,0.5,maxRuns+0.5);
    hV0MMultRun[itrigger] = new TH2F(Form("hV0MMultRun_%s",trigName.Data()),Form("%s V0M multiplicity per run",trigName.Data()),maxRuns,0.5,maxRuns+0.5,100,0,800);
    hTKLMultRun[itrigger] = new TH2F(Form("hTKLMultRun_%s",trigName.Data()),Form("%s TKL multiplicity per run",trigName.Data()),maxRuns,0.5,maxRuns+0.5,100,0,200);
    // percentiles per run
    hV0MCentRun[itrigger] = new TH2F(Form("hV0MCentRun_%s",trigName.Data()),Form("%s V0M M/<M> per run",trigName.Data()),maxRuns,0.5,maxRuns+0.5,100,0,10);
    hTKLCentRun[itrigger] = new TH2F(Form("hTKLCentRun_%s",trigName.Data()),Form("%s TKL M/<M> per run",trigName.Data()),maxRuns,0.5,maxRuns+0.5,100,0,10);
    // vertex QA
    hVzRun[itrigger] = new TH2F(Form("hVzRun_%s",trigName.Data()),Form("%s vz per run;run number;vz",trigName.Data()),maxRuns,0.5,maxRuns+0.5,90,zmin,zmax);
    hVzV0MCent[itrigger] = new TH2F(Form("hVzV0MCent_%s",trigName.Data()),Form("%s vz vs V0M M/<M>;vz;V0M M/<M>",trigName.Data()),45,zmin,zmax,20,0,10);
    hVzTKLCent[itrigger] = new TH2F(Form("hVzTKLCent_%s",trigName.Data()),Form("%s vz vs TKL M/<M>;vz;TKL M/<M>",trigName.Data()),45,zmin,zmax,20,0,10);
    // check distributions of V0M and TKL estimators
    hNeventsV0M[itrigger] = new TH1D(Form("hNeventsV0M_%s",trigName.Data()),Form("%s;V0M M/<M>;Nevents",trigName.Data()),100,0,10);
    hNeventsTKL[itrigger] = new TH1D(Form("hNeventsTKL_%s",trigName.Data()),Form("%s;TKL M/<M>;Nevents",trigName.Data()),100,0,10);
    // check multiplicity reach
    hV0MNch[itrigger] = new TH2D(Form("hV0MNch_%s",trigName.Data()),Form("%s;V0M M/<M>;Nch in TPC",trigName.Data()),50,0,10,100,0,100);
    hTKLNch[itrigger] = new TH2D(Form("hTKLNch_%s",trigName.Data()),Form("%s;TKL M/<M>;Nch in TPC",trigName.Data()),50,0,10,100,0,100);
    // check correlation with percentiles
    hV0MCentPerc[itrigger] = new TH2F(Form("hV0MCentPerc_%s",trigName.Data()),Form("%s V0M M/<M> versus percentile;M/<M>;multiplicity percentile",trigName.Data()),100,0,10,100,0,100);
    hV0MCentPercLog[itrigger] = new TH2F(Form("hV0MCentPercLog_%s",trigName.Data()),Form("%s V0M M/<M> versus percentile;M/<M>;log(multiplicity percentile)",trigName.Data()),100,0,10,100,-3,2);

    // check SPD vertex vs primary vertex
    hVertexSPDprim[itrigger] = new TH2D(Form("hVertexSPDprim_%s",trigName.Data()),Form("%s primary vertex vs SPD vertex;primary z-vertex;SPD z-vertex",trigName.Data()),100,-10,10,100,-10,10);
    hVertexSPDprimMult[itrigger] = new TH2D(Form("hVertexSPDprimMult_%s",trigName.Data()),Form("%s primary vertex vs SPD vertex vs V0M multiplicity;primary - SPD vertex;V0M M/<M>",trigName.Data()),200,-5,5,100,0,10);
    hVertexResMult[itrigger] = new TH2D(Form("hVertexResMult_%s",trigName.Data()),Form("%s vertex resolution vs V0M multiplicity;SPD vertex resolution;V0M M/<M>",trigName.Data()),200,0,10,100,0,10);
    hVertexContribMult[itrigger] = new TH2D(Form("hVertexContribMult_%s",trigName.Data()),Form("%s vertex contributors vs V0M multiplicity;SPD vertex resolution;V0M M/<M>",trigName.Data()),71,-0.5,70.5,100,0,10);
    // check mean multiplicity calibration
    hUncorr_V0M[itrigger] = new TH2D(Form("hUncorr_V0M_%s",trigName.Data()),Form("%s uncorrected multiplicity",trigName.Data()),maxRuns,0,maxRuns,400,0,800);
    hCorr_V0M[itrigger] = new TH2D(Form("hCorr_V0M_%s",trigName.Data()),Form("%s corrected mean multiplicity",trigName.Data()),maxRuns,0,maxRuns,400,0,10);
    hUncorr_TKL[itrigger] = new TH2D(Form("hUncorr_TKL_%s",trigName.Data()),Form("%s uncorrected multiplicity",trigName.Data()),maxRuns,0,maxRuns,100,0,200);
    hCorr_TKL[itrigger] = new TH2D(Form("hCorr_TKL_%s",trigName.Data()),Form("%s corrected mean multiplicity",trigName.Data()),maxRuns,0,maxRuns,100,0,10);
    hVzNch[itrigger] = new TH2D(Form("hVzNch_%s",trigName.Data()),Form("%s vz versus Nch",trigName.Data()),50,0,50,91,zmin,zmax);

    // from Leonardo
    hAllEvents_V0M[itrigger] = new TH1D(Form("hAllEvents_V0M_%s",trigName.Data()),"",200,0,800);
    hAllEvents_TKL[itrigger] = new TH1D(Form("hAllEvents_TKL_%s",trigName.Data()),"",50,0,200);
    hPileupSPD_V0M[itrigger] = new TH1D(Form("hPileupSPD_V0M_%s",trigName.Data()),"",200,0,800);
    hPileupSPD_TKL[itrigger] = new TH1D(Form("hPileupSPD_TKL_%s",trigName.Data()),"",50,0,200);
    hAllEvents_TklMult_dphi_V0M[itrigger] = new TH3D(Form("hAllEvents_TklMult_dphi_V0M_%s",trigName.Data()),"All Events;Ntkl;dphi;V0M M/<M>",101,-0.5,100.5,5,0,5,50,0,10);
    hAllEvents_TklMult_dphi_TKL[itrigger] = new TH3D(Form("hAllEvents_TklMult_dphi_TKL_%s",trigName.Data()),"All Events;Ntkl;dphi;TKL M/<M>",101,-0.5,100.5,5,0,5,50,0,10);
    hSingleDiffractive_TklMult_dphi_V0M[itrigger] = new TH3D(Form("hSingleDiffractive_TklMult_dphi_V0M_%s",trigName.Data()),"Single Diffraction;Ntkl;dphi;V0M M/<M>",101,-0.5,100.5,5,0,5,50,0,10);
    hSingleDiffractive_TklMult_dphi_TKL[itrigger] = new TH3D(Form("hSingleDiffractive_TklMult_dphi_TKL_%s",trigName.Data()),"Single Diffraction;Ntkl;dphi;TKL M/<M>",101,-0.5,100.5,5,0,5,50,0,10);
    hDoubleDiffractive_TklMult_dphi_V0M[itrigger] = new TH3D(Form("hDoubleDiffractive_TklMult_dphi_V0M_%s",trigName.Data()),"Double Diffraction;Ntkl;dphi;V0M M/<M>",101,-0.5,100.5,5,0,5,50,0,10);
    hDoubleDiffractive_TklMult_dphi_TKL[itrigger] = new TH3D(Form("hDoubleDiffractive_TklMult_dphi_TKL_%s",trigName.Data()),"Double Diffraction;Ntkl;dphi;TKL M/<M>",101,-0.5,100.5,5,0,5,50,0,10);
    hAllEvents_TrkMult_pt_V0M[itrigger] = new TH3D(Form("hAllEvents_TrkMult_pt_V0M_%s",trigName.Data()),"All Events;Ntrk;pt;V0M M/<M>",101,-0.5,100.5,5,0,2.5,50,0,10);
    hAllEvents_TrkMult_pt_TKL[itrigger] = new TH3D(Form("hAllEvents_TrkMult_pt_TKL_%s",trigName.Data()),"All Events;Ntrk;pt;TKL M/<M>",101,-0.5,100.5,5,0,2.5,50,0,10);
    hSingleDiffractive_TrkMult_pt_V0M[itrigger] = new TH3D(Form("hSingleDiffractive_TrkMult_pt_V0M_%s",trigName.Data()),"Single Diffraction;Ntrk;pt;V0M M/<M>",101,-0.5,100.5,5,0,2.5,50,0,10);
    hSingleDiffractive_TrkMult_pt_TKL[itrigger] = new TH3D(Form("hSingleDiffractive_TrkMult_pt_TKL_%s",trigName.Data()),"Single Diffraction;Ntrk;pt;TKL M/<M>",101,-0.5,100.5,5,0,2.5,50,0,10);
    hDoubleDiffractive_TrkMult_pt_V0M[itrigger] = new TH3D(Form("hDoubleDiffractive_TrkMult_pt_V0M_%s",trigName.Data()),"Double Diffraction;Ntrk;pt;V0M M/<M>",101,-0.5,100.5,5,0,2.5,50,0,10);
    hDoubleDiffractive_TrkMult_pt_TKL[itrigger] = new TH3D(Form("hDoubleDiffractive_TrkMult_pt_TKL_%s",trigName.Data()),"Double Diffraction;Ntrk;pt;TKL M/<M>",101,-0.5,100.5,5,0,2.5,50,0,10);
    hPileupMV_V0M[itrigger] = new TH1D(Form("hPileupMV_V0M_%s",trigName.Data()),"",200,0,800);
    hPileupMV_TKL[itrigger] = new TH1D(Form("hPileupMV_TKL_%s",trigName.Data()),"",50,0,200);
    hTrackClusterCut_V0M[itrigger] = new TH1D(Form("hTrackClusterCut_V0M_%s",trigName.Data()),"",200,0,800);
    hTrackClusterCut_TKL[itrigger] = new TH1D(Form("hTrackClusterCut_TKL_%s",trigName.Data()),"",50,0,200);
    hOutOfBunchPileup_V0M[itrigger] = new TH1D(Form("hOutOfBunchPileup_V0M_%s",trigName.Data()),"",200,0,800);
    hOutOfBunchPileup_TKL[itrigger] = new TH1D(Form("hOutOfBunchPileup_TKL_%s",trigName.Data()),"",50,0,200);
    hV0Asymmetry_V0M[itrigger] = new TH1D(Form("hV0Asymmetry_V0M_%s",trigName.Data()),"",200,0,800);
    hV0Asymmetry_TKL[itrigger] = new TH1D(Form("hV0Asymmetry_TKL_%s",trigName.Data()),"",50,0,200);
    hV0Decision_V0M[itrigger] = new TH1D(Form("hV0Decision_V0M_%s",trigName.Data()),"",200,0,800);
    hV0Decision_TKL[itrigger] = new TH1D(Form("hV0Decision_TKL_%s",trigName.Data()),"",50,0,200);
    hIncompleteEvents_V0M[itrigger] = new TH1D(Form("hIncompleteEvents_V0M_%s",trigName.Data()),"",200,0,800);
    hIncompleteEvents_TKL[itrigger] = new TH1D(Form("hIncompleteEvents_TKL_%s",trigName.Data()),"",50,0,200);
    hTrackClusterCorr[itrigger] = new TH2D(Form("hTrackClusterCorr_%s",trigName.Data()),Form("hTrackClusterCorr_%s;SPD tracklets;SPD clusters",trigName.Data()),200,0,200,200,0,2000);
    hTrackClusterCorrCut[itrigger] = new TH2D(Form("hTrackClusterCorrCut_%s",trigName.Data()),Form("hTrackClusterCorrCut_%s;SPD tracklets;SPD clusters",trigName.Data()),200,0,200,200,0,2000);
    hOnlineOfflineFastOR[itrigger] = new TH2D(Form("hOnlineOfflineFastOR_%s",trigName.Data()),Form("hOnlineOfflineFastOR_%s;Online fastOR;Offline fastOR",trigName.Data()),200,0,200,200,0,200);
    hOnlineOfflineFastORCut[itrigger] = new TH2D(Form("hOnlineOfflineFastORCut_%s",trigName.Data()),Form("hOnlineOfflineFastORCut_%s;Online fastOR;Offline fastOR",trigName.Data()),200,0,200,200,0,200);
    hV0AsymmCorr[itrigger] = new TH2D(Form("hV0AsymmCorr_%s",trigName.Data()),Form("hV0AsymmCorr_%s;V0C_3;V0C_012",trigName.Data()),200,0,200,200,0,2000);
    hV0AsymmCorrCut[itrigger] = new TH2D(Form("hV0AsymmCorrCut_%s",trigName.Data()),Form("hV0AsymmCorrCut_%s;V0C_3;V0C_012",trigName.Data()),200,0,200,200,0,2000);
    h_V0M_TKL[itrigger] = new TH2F(Form("h_V0M_TKL_%s",trigName.Data()),Form("%s triggered events;V0M multiplicity;TKL multiplicity",trigName.Data()),200,0,800,200,0,800);
    hVz_V0M[itrigger] = new TH2F(Form("hVz_V0M_%s",trigName.Data()),"vz vs V0M multiplicity;vz;V0M multiplicity",100,-10,10,160,0,800);
    hVz_TKL[itrigger] = new TH2F(Form("hVz_TKL_%s",trigName.Data()),"vz vs TKL multiplicity;vz;TKL multiplicity",100,-10,10,160,0,800);
  }

  for (Int_t ev=0;ev<nEvents;ev++){
    //  for (Int_t ev=0;ev<1000;ev++){
    if (ev%100000==0) Printf("Event=%i   %.2lf%% done",ev,Double_t(ev)/Double_t(nEvents)*100.);
    fTree->GetEntry(ev);

    if(fPrevRun != fCurrentRunNumber)
    {
      fPrevRun = fCurrentRunNumber;
      countRuns  = runindex[fCurrentRunNumber];
      cout << countRuns << "   " << fCurrentRunNumber << endl;
      //      if(multFile->Get(Form("hMult%s_%i","V0M",fCurrentRunNumber)))
      //      {
      //        meanMultV0M = ((TH1D*)multFile->Get(Form("hMult%s_%i","V0M",fCurrentRunNumber)))->GetMean();
      //        meanMultV0A = ((TH1D*)multFile->Get(Form("hMult%s_%i","V0A",fCurrentRunNumber)))->GetMean();
      //        meanMultV0C = ((TH1D*)multFile->Get(Form("hMult%s_%i","V0C",fCurrentRunNumber)))->GetMean();
      //        meanMultTKL = ((TH1D*)multFile->Get(Form("hMult%s_%i","TKL",fCurrentRunNumber)))->GetMean();
      //
      //        if(multFile->Get(Form("hMultV0Mmc_%i",fCurrentRunNumber)))
      //        {
      //          meanMultV0Mmc = ((TH1D*)multFile->Get(Form("hMult%smc_%i","V0M",fCurrentRunNumber)))->GetMean();
      //          meanMultV0Amc = ((TH1D*)multFile->Get(Form("hMult%smc_%i","V0A",fCurrentRunNumber)))->GetMean();
      //          meanMultV0Cmc = ((TH1D*)multFile->Get(Form("hMult%smc_%i","V0C",fCurrentRunNumber)))->GetMean();
      //          meanMultTKLmc = ((TH1D*)multFile->Get(Form("hMult%smc_%i","TPC",fCurrentRunNumber)))->GetMean();
      //        }

      Printf("mean V0M multiplicity for run %i is %.2lf",fCurrentRunNumber,fMultMeanV0M);
      Printf("mean TKL multiplicity for run %i is %.2lf",fCurrentRunNumber,fMultMeanTKL);

      //      if(meanMultV0Mmc > -0.5)
      //      {
      //        Printf("mean mc V0M multiplicity for run %i is %.2lf",fCurrentRunNumber,meanMultV0Mmc);
      //        Printf("mean mc V0A multiplicity for run %i is %.2lf",fCurrentRunNumber,meanMultV0Amc);
      //        Printf("mean mc V0C multiplicity for run %i is %.2lf",fCurrentRunNumber,meanMultV0Cmc);
      //        Printf("mean mc TKL multiplicity for run %i is %.2lf",fCurrentRunNumber,meanMultTKLmc);
      //      }

      for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++){
        hAllEventsRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
        hEventsRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));

        hV0MMultRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
        hTKLMultRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));

        hV0MCentRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
        hTKLCentRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));

        hVzRun[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));

        hUncorr_V0M[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
        hCorr_V0M[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
        hUncorr_TKL[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
        hCorr_TKL[itrigger]->GetXaxis()->SetBinLabel(countRuns,Form("%i",fCurrentRunNumber));
      }
    }

    //******************************************************************************
    // from Leonardo's QA macro
    //loop over triggers
    for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++){
      if(itrigger == 0 && !(fSelectMask & AliVEvent::kMB)) continue;
      else if(itrigger > 0 && !(fClassesFired & 1<<(itrigger-1))) continue;


      //all events
      hAllEvents_V0M[itrigger]->Fill(fMultV0Meq);
      hAllEvents_TKL[itrigger]->Fill(fMultTKL);

      //out of bunch 11 BC
      if (IsOutOfBunchPileup()){
        hOutOfBunchPileup_V0M[itrigger]->Fill(fMultV0Meq);
        hOutOfBunchPileup_TKL[itrigger]->Fill(fMultTKL);
      }

      //V0 Asymmetry cut
      hV0AsymmCorr[itrigger]->Fill(fMultV0Cring[3],fMultV0Cring[0] + fMultV0Cring[1] + fMultV0Cring[2]);
      if (IsAsymmetricV0()){
        hV0Asymmetry_V0M[itrigger]->Fill(fMultV0Meq);
        hV0Asymmetry_TKL[itrigger]->Fill(fMultTKL);
      }else hV0AsymmCorrCut[itrigger]->Fill(fMultV0Cring[3],fMultV0Cring[0] + fMultV0Cring[1] + fMultV0Cring[2]);


      // V0 decision
      if(fV0CDecision!=1 || fV0ADecision!=1){
        hV0Decision_V0M[itrigger]->Fill(fMultV0Meq);
        hV0Decision_TKL[itrigger]->Fill(fMultTKL);
      }

      hTrackClusterCorr[itrigger]->Fill(fMultTKL,fNofITSClusters[0]+fNofITSClusters[1]);
      //track-cluster cut
      if (fNofITSClusters[0]+fNofITSClusters[1]>64+4*fMultTKL){
        hTrackClusterCut_V0M[itrigger]->Fill(fMultV0Meq);
        hTrackClusterCut_TKL[itrigger]->Fill(fMultTKL);
      }else hTrackClusterCorrCut[itrigger]->Fill(fMultTKL,fNofITSClusters[0]+fNofITSClusters[1]);

      //fill online-offline fastor cut
      hOnlineOfflineFastOR[itrigger]->Fill(fonlineSPD,fofflineSPD);
      if(!(fonlineSPD <= -20.589 + 0.73664*fofflineSPD))hOnlineOfflineFastORCut[itrigger]->Fill(fonlineSPD,fofflineSPD);

      //SPD pileup
      if(fIsPileupSPD){
        hPileupSPD_V0M[itrigger]->Fill(fMultV0Meq);
        hPileupSPD_TKL[itrigger]->Fill(fMultTKL);
      }
      //MV pileup
      if(fIsPileupMV){
        hPileupMV_V0M[itrigger]->Fill(fMultV0Meq);
        hPileupMV_TKL[itrigger]->Fill(fMultTKL);
      }
      //if(fIsPileupMV || fIsPileupSPD) continue;

      //incomplete events
      if (fIsIncomplete){
        hIncompleteEvents_V0M[itrigger]->Fill(fMultV0Meq);
        hIncompleteEvents_TKL[itrigger]->Fill(fMultTKL);
      }

    }

    //*****************************************************************************
    // M/<M>
    Float_t centV0M = 0, centTKL = 0;
    centV0M = Double_t(fMultV0Meq)/fMultMeanV0M;
    centTKL = Double_t(fMultTKL)/fMultMeanTKL;

    //  Double_t centV0Mmc = 0, centV0Amc = 0, centV0Cmc = 0, centTKLmc = 0;
    //  if(fNchV0Amc > -0.5)
    //  {
    //    centV0Amc = Double_t(fNchV0Amc)/meanMultV0Amc;
    //    centV0Cmc = Double_t(fNchV0Cmc)/meanMultV0Cmc;
    //    centV0Mmc = Double_t(fNchV0Amc+fNchV0Cmc)/meanMultV0Mmc;
    //    centTKLmc = Double_t(fNchTPCmc)/meanMultTKLmc;
    //  }

    Bool_t eventSel = 0;
    Bool_t isMC=0;
    if(anaType==kTrkTrkGen || anaType==kTklTklGen || anaType==kTrkTrkITSGen)isMC=1;
    if(fMcParticles)isMC=1; //this is needed to run the same analysis as in the data skipping the pileup selection
    //HM events statistics
    if(energy==k13TeV) eventSel = IsGoodEvent13TeV(AliVEvent::kAny,isMC ? 0x0 : 1<<k_CVHMV0M_B,zmin,zmax,centV0M,hEventCount_CVHMV0M_B,isMC);

    //event slection (selects all triggers)
    if(energy==k13TeV) eventSel = IsGoodEvent13TeV(AliVEvent::kAny,isMC ? 0x0 : alltriggerclasses13TeV,zmin,zmax,centV0M,hEventCount,isMC);
    else eventSel = IsGoodEvent7TeV(AliVEvent::kMB,(anaType==kTrkTrkGen || anaType==kTklTklMC) ? 0x0 : 0xffffffff,zmin,zmax,centV0M,hEventCount);

    if (!eventSel) continue;

    // loop over triggers
    for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++){
      if(itrigger == 0 && !(fSelectMask & AliVEvent::kMB)) continue;
      if(itrigger > 0 && !(fClassesFired & 1<<(itrigger-1))) continue;


      //trigger stat
      hTriggerStat_V0M->Fill(itrigger,fMultV0Meq);
      hTriggerStat_TKL->Fill(itrigger,fMultTKL);


      //todo change with mult/<mult>
      h_V0M_TKL[itrigger]->Fill(fMultV0Meq,fMultTKL);
      hVz_V0M[itrigger]->Fill(fSPDVtxZ,fMultV0Meq);
      hVz_TKL[itrigger]->Fill(fSPDVtxZ,fMultTKL);

      //*****************************************************************************
      hAllEventsV0M[itrigger]->Fill(fMultV0Meq);
      hAllEventsRun[itrigger]->Fill(countRuns);

      hUncorr_V0M[itrigger]->Fill((Double_t)countRuns-0.5,fMultV0Meq);
      hUncorr_TKL[itrigger]->Fill((Double_t)countRuns-0.5,fMultTKL);
      hCorr_V0M[itrigger]->Fill((Double_t)countRuns-0.5,(fMultV0Meq)/fMultMeanV0M);
      hCorr_TKL[itrigger]->Fill((Double_t)countRuns-0.5,(fMultTKL)/fMultMeanTKL);

      //*****************************************************************************
      // M/<M> correlation check
      hV0MTKL[itrigger]->Fill(centV0M,centTKL);

      hV0AV0CMult[itrigger]->Fill(fMultV0Aeq,fMultV0Ceq);

      //*****************************************************************************
      // multiplicities and percentiles per run
      hEventsRun[itrigger]->Fill(countRuns);

      hV0MMultRun[itrigger]->Fill(countRuns,fMultV0Meq);
      hTKLMultRun[itrigger]->Fill(countRuns,fMultTKL);

      hV0MCentRun[itrigger]->Fill(countRuns,centV0M);
      hTKLCentRun[itrigger]->Fill(countRuns,centTKL);

      //*****************************************************************************
      // check distributions of V0M and TKL estimators
      hNeventsV0M[itrigger]->Fill(centV0M);
      hNeventsTKL[itrigger]->Fill(centTKL);

      //*****************************************************************************
      if(anaType == kTklTklMC){
        //diffractive - tkl
        if(fTracklets){
          //get dphi distr to get the mult in a given range
          TH1D *hdphidistr=hAllEvents_TklMult_dphi_V0M[itrigger]->ProjectionY("hdphidistr");
          hdphidistr->Reset("all");
          for (Int_t i=0;i<fTracklets->GetEntriesFast();i++){
            AliCFParticle* track = (AliCFParticle*) fTracklets->UncheckedAt(i);
            Float_t dphi =TMath::Abs(track->Pt())*1000;
            hdphidistr->Fill(dphi);
          }
          //for each range fill the mult
          for(Int_t ibindphi=1;ibindphi<= hAllEvents_TklMult_dphi_V0M[itrigger]->GetYaxis()->GetNbins();ibindphi++){
            //count the mult up to a given dphi value 0->dphi
            Double_t mult=hdphidistr->Integral(1,ibindphi);
            Double_t dphi=hAllEvents_TklMult_dphi_V0M[itrigger]->GetYaxis()->GetBinCenter(ibindphi);

            hAllEvents_TklMult_dphi_V0M[itrigger]->Fill(mult,dphi,centV0M);
            hAllEvents_TklMult_dphi_TKL[itrigger]->Fill(mult,dphi,centTKL);
            if(fIsDiffractive==1){
              hSingleDiffractive_TklMult_dphi_V0M[itrigger]->Fill(mult,dphi,centV0M);
              hSingleDiffractive_TklMult_dphi_TKL[itrigger]->Fill(mult,dphi,centTKL);
            }else if(fIsDiffractive==2){
              hDoubleDiffractive_TklMult_dphi_V0M[itrigger]->Fill(mult,dphi,centV0M);
              hDoubleDiffractive_TklMult_dphi_TKL[itrigger]->Fill(mult,dphi,centTKL);
            }
          }
        }

        //diffractive - trk
        if(fMcParticles){
          //get pt distr to get the mult in a given range
          TH1D *hptdistr=hAllEvents_TrkMult_pt_V0M[itrigger]->ProjectionY("hptdistr");
          hptdistr->Reset("all");
          for (Int_t i=0;i<fMcParticles->GetEntriesFast();i++){
            AliCFParticle* track = (AliCFParticle*) fMcParticles->UncheckedAt(i);
            //          if(track->Pt()<0.1)Printf("pt   %f",track->Pt());
            hptdistr->Fill(track->Pt());
          }
          //for each range fill the mult
          for(Int_t ibinpt=1;ibinpt<= hAllEvents_TrkMult_pt_V0M[itrigger]->GetYaxis()->GetNbins();ibinpt++){
            //count the mult above a given pt (including overflow)
            Double_t mult=hptdistr->Integral(ibinpt,hAllEvents_TrkMult_pt_V0M[itrigger]->GetYaxis()->GetNbins()+1);
            Double_t pt=hAllEvents_TrkMult_pt_V0M[itrigger]->GetYaxis()->GetBinCenter(ibinpt);

            hAllEvents_TrkMult_pt_V0M[itrigger]->Fill(mult,pt,centV0M);
            hAllEvents_TrkMult_pt_TKL[itrigger]->Fill(mult,pt,centTKL);
            if(fIsDiffractive==1){
              hSingleDiffractive_TrkMult_pt_V0M[itrigger]->Fill(mult,pt,centV0M);
              hSingleDiffractive_TrkMult_pt_TKL[itrigger]->Fill(mult,pt,centTKL);
            }else if(fIsDiffractive==2){
              hDoubleDiffractive_TrkMult_pt_V0M[itrigger]->Fill(mult,pt,centV0M);
              hDoubleDiffractive_TrkMult_pt_TKL[itrigger]->Fill(mult,pt,centTKL);
            }
          }
        }
      }

      //*****************************************************************************
      // vertex QA
      hVzRun[itrigger]->Fill(countRuns,fVtxZ);
      hVzV0MCent[itrigger]->Fill(fVtxZ,centV0M);
      hVzTKLCent[itrigger]->Fill(fVtxZ,centTKL);
      hVzNch[itrigger]->Fill(fNchTPC,fVtxZ);

      //*****************************************************************************
      // check multiplicity reach
      hV0MNch[itrigger]->Fill(centV0M,fNchTPC);
      hTKLNch[itrigger]->Fill(centTKL,fNchTPC);

      hVertexSPDprim[itrigger]->Fill(fVtxZ,fSPDVtxZ);
      hVertexSPDprimMult[itrigger]->Fill(fVtxZ-fSPDVtxZ,centV0M);
      hVertexResMult[itrigger]->Fill(fSPDVtxZRes,centV0M);
      hVertexContribMult[itrigger]->Fill(fVtxContributors,centV0M);
      //      if(TMath::Abs(fVtxZ)<10e-9){
      //        Printf("fVtxZ %f",fVtxZ);
      //        Printf("fSPDVtxZ %f",fSPDVtxZ);
      //        Printf("fVtxContributors %u",fVtxContributors);
      //        Printf("fSPDVtxContributors %u",fSPDVtxContributors);
      //        Printf("");
      //      }
      //*****************************************************************************
      // correlation between percentiles and M/<M>
      hV0MCentPerc[itrigger]->Fill(centV0M,fMultPercV0Meq);
      if(fMultPercV0Meq > 0)
        hV0MCentPercLog[itrigger]->Fill(centV0M,TMath::Log10(fMultPercV0Meq));
    }
    //if(ev>500000) break;
    //if(countRuns > 25) break;

  }
  //hAllEventsV0M->Sumw2();
  //hAllEventsRun->Sumw2();

  TFile *fout = new TFile(Form("eventQA_%s.root",period),"recreate");

  hTriggerStat_V0M->Write();
  hTriggerStat_TKL->Write();
  hEventCount->Write();
  hEventCount_CVHMV0M_B->Write();

  TH1D* hTriggerStat = (TH1D*)hTriggerStat_V0M->ProjectionX();

  for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++){
    if(hTriggerStat->GetBinContent(itrigger+1) == 0) continue;

    hAllEventsV0M[itrigger]->Write();
    hAllEventsRun[itrigger]->Write();
    hV0MTKL[itrigger]->Write();
    hV0AV0CMult[itrigger]->Write();
    hEventsRun[itrigger]->Write();
    hV0MMultRun[itrigger]->Write();
    hTKLMultRun[itrigger]->Write();
    hV0MCentRun[itrigger]->Write();
    hTKLCentRun[itrigger]->Write();
    hVzRun[itrigger]->Write();
    hVzV0MCent[itrigger]->Write();
    hVzTKLCent[itrigger]->Write();
    hNeventsV0M[itrigger]->Write();
    hNeventsTKL[itrigger]->Write();
    hV0MNch[itrigger]->Write();
    hTKLNch[itrigger]->Write();
    hV0MCentPerc[itrigger]->Write();
    hV0MCentPercLog[itrigger]->Write();
    hVertexSPDprim[itrigger]->Write();
    hVertexSPDprimMult[itrigger]->Write();
    hVertexResMult[itrigger]->Write();
    hVertexContribMult[itrigger]->Write();
    hUncorr_V0M[itrigger]->Write();
    hUncorr_TKL[itrigger]->Write();
    hCorr_V0M[itrigger]->Write();
    hCorr_TKL[itrigger]->Write();
    hVzNch[itrigger]->Write();

    hAllEvents_V0M[itrigger]->Write();
    hAllEvents_TKL[itrigger]->Write();
    hAllEvents_TklMult_dphi_V0M[itrigger]->Write();
    hAllEvents_TklMult_dphi_TKL[itrigger]->Write();
    hSingleDiffractive_TklMult_dphi_V0M[itrigger]->Write();
    hSingleDiffractive_TklMult_dphi_TKL[itrigger]->Write();
    hDoubleDiffractive_TklMult_dphi_V0M[itrigger]->Write();
    hDoubleDiffractive_TklMult_dphi_TKL[itrigger]->Write();
    hAllEvents_TrkMult_pt_V0M[itrigger]->Write();
    hAllEvents_TrkMult_pt_TKL[itrigger]->Write();
    hSingleDiffractive_TrkMult_pt_V0M[itrigger]->Write();
    hSingleDiffractive_TrkMult_pt_TKL[itrigger]->Write();
    hDoubleDiffractive_TrkMult_pt_V0M[itrigger]->Write();
    hDoubleDiffractive_TrkMult_pt_TKL[itrigger]->Write();
    hPileupSPD_V0M[itrigger]->Write();
    hPileupSPD_TKL[itrigger]->Write();
    hPileupMV_V0M[itrigger]->Write();
    hPileupMV_TKL[itrigger]->Write();
    hTrackClusterCut_V0M[itrigger]->Write();
    hTrackClusterCut_TKL[itrigger]->Write();
    hOutOfBunchPileup_V0M[itrigger]->Write();
    hOutOfBunchPileup_TKL[itrigger]->Write();
    hV0Asymmetry_V0M[itrigger]->Write();
    hV0Asymmetry_TKL[itrigger]->Write();
    hV0Decision_V0M[itrigger]->Write();
    hV0Decision_TKL[itrigger]->Write();
    hIncompleteEvents_V0M[itrigger]->Write();
    hIncompleteEvents_TKL[itrigger]->Write();
    hTrackClusterCorr[itrigger]->Write();
    hTrackClusterCorrCut[itrigger]->Write();
    hOnlineOfflineFastOR[itrigger]->Write();
    hOnlineOfflineFastORCut[itrigger]->Write();
    hV0AsymmCorr[itrigger]->Write();
    hV0AsymmCorrCut[itrigger]->Write();
    h_V0M_TKL[itrigger]->Write();
    hVz_V0M[itrigger]->Write();
    hVz_TKL[itrigger]->Write();
  }

  fout->Close();

  //  for(Int_t itrigger=0;itrigger<maxTrigs;itrigger++)
  //    drawEventQA(Form("eventQA_%s.root",period),itrigger);
  drawEventQA(Form("eventQA_%s.root",period),4);
}


void drawEventQA(TString filename, Int_t itrigger)
{
  //  gStyle->SetOptStat(0);//111111);
  gStyle->SetOptStat(111111);

  TString trigName;
  if(itrigger == 0) trigName = "kMB";
  else trigName = TriggerName[itrigger-1];

  TFile *fin = new TFile(filename,"read");

  TH1D* hAllEventsV0M = (TH1D*)fin->Get("hAllEventsV0M");
  TH1D* hAllEventsRun = (TH1D*)fin->Get("hAllEventsRun");
  TH3D* hEventCount = (TH3D*)fin->Get("hEventCount");
  TH3D* hEventCount_CVHMV0M_B = (TH3D*)fin->Get("hEventCount_CVHMV0M_B");
  TH2D* hTriggerStat_V0M = (TH2D*)fin->Get(Form("hTriggerStat_V0M"));
  TH2D* hTriggerStat_TKL = (TH2D*)fin->Get(Form("hTriggerStat_TKL"));

  TCanvas *cTriggerStat=new TCanvas("cTriggerStat","cTriggerStat",1200,800);
  cTriggerStat->Divide(2,1);
  cTriggerStat->cd(1);
  gPad->SetLogz();
  gPad->SetRightMargin(0.15);
  hTriggerStat_V0M->DrawCopy("colz");
  cTriggerStat->cd(2);
  gPad->SetLogz();
  gPad->SetRightMargin(0.15);
  hTriggerStat_TKL->DrawCopy("colz");
  cTriggerStat->Update();

  TCanvas *cTriggerStatInt=new TCanvas("cTriggerStatInt","cTriggerStatInt",1200,500);
  cTriggerStatInt->cd(1);
  gPad->SetLogy();
  gPad->SetGridy();
  TH1D* hTriggerStat = (TH1D*)hTriggerStat_V0M->ProjectionX();
  hTriggerStat->DrawCopy("")->SetFillColor(4);
  cTriggerStatInt->Update();
  if(hTriggerStat->GetBinContent(itrigger+1) == 0){
    cout << "No histograms for trigger " << trigName << endl;
    return;
  }

  TH1D* hAllEvents_V0M= (TH1D*)fin->Get(Form("hAllEvents_V0M_%s",trigName.Data()));
  TH1D* hAllEvents_TKL= (TH1D*)fin->Get(Form("hAllEvents_TKL_%s",trigName.Data()));
  TH3D* hAllEvents_TklMult_dphi_V0M= (TH3D*)fin->Get(Form("hAllEvents_TklMult_dphi_V0M_%s",trigName.Data()));
  TH3D* hAllEvents_TklMult_dphi_TKL= (TH3D*)fin->Get(Form("hAllEvents_TklMult_dphi_TKL_%s",trigName.Data()));
  TH3D* hSingleDiffractive_TklMult_dphi_V0M= (TH3D*)fin->Get(Form("hSingleDiffractive_TklMult_dphi_V0M_%s",trigName.Data()));
  TH3D* hSingleDiffractive_TklMult_dphi_TKL= (TH3D*)fin->Get(Form("hSingleDiffractive_TklMult_dphi_TKL_%s",trigName.Data()));
  TH3D* hDoubleDiffractive_TklMult_dphi_V0M= (TH3D*)fin->Get(Form("hDoubleDiffractive_TklMult_dphi_V0M_%s",trigName.Data()));
  TH3D* hDoubleDiffractive_TklMult_dphi_TKL= (TH3D*)fin->Get(Form("hDoubleDiffractive_TklMult_dphi_TKL_%s",trigName.Data()));
  TH3D* hAllEvents_TrkMult_pt_V0M= (TH3D*)fin->Get(Form("hAllEvents_TrkMult_pt_V0M_%s",trigName.Data()));
  TH3D* hAllEvents_TrkMult_pt_TKL= (TH3D*)fin->Get(Form("hAllEvents_TrkMult_pt_TKL_%s",trigName.Data()));
  TH3D* hSingleDiffractive_TrkMult_pt_V0M= (TH3D*)fin->Get(Form("hSingleDiffractive_TrkMult_pt_V0M_%s",trigName.Data()));
  TH3D* hSingleDiffractive_TrkMult_pt_TKL= (TH3D*)fin->Get(Form("hSingleDiffractive_TrkMult_pt_TKL_%s",trigName.Data()));
  TH3D* hDoubleDiffractive_TrkMult_pt_V0M= (TH3D*)fin->Get(Form("hDoubleDiffractive_TrkMult_pt_V0M_%s",trigName.Data()));
  TH3D* hDoubleDiffractive_TrkMult_pt_TKL= (TH3D*)fin->Get(Form("hDoubleDiffractive_TrkMult_pt_TKL_%s",trigName.Data()));
  TH1D* hPileupSPD_V0M= (TH1D*)fin->Get(Form("hPileupSPD_V0M_%s",trigName.Data()));
  TH1D* hPileupSPD_TKL= (TH1D*)fin->Get(Form("hPileupSPD_TKL_%s",trigName.Data()));
  TH1D* hPileupMV_V0M= (TH1D*)fin->Get(Form("hPileupMV_V0M_%s",trigName.Data()));
  TH1D* hPileupMV_TKL= (TH1D*)fin->Get(Form("hPileupMV_TKL_%s",trigName.Data()));
  TH1D* hTrackClusterCut_V0M= (TH1D*)fin->Get(Form("hTrackClusterCut_V0M_%s",trigName.Data()));
  TH1D* hTrackClusterCut_TKL= (TH1D*)fin->Get(Form("hTrackClusterCut_TKL_%s",trigName.Data()));
  TH1D* hOutOfBunchPileup_V0M= (TH1D*)fin->Get(Form("hOutOfBunchPileup_V0M_%s",trigName.Data()));
  TH1D* hOutOfBunchPileup_TKL= (TH1D*)fin->Get(Form("hOutOfBunchPileup_TKL_%s",trigName.Data()));
  TH1D* hV0Asymmetry_V0M= (TH1D*)fin->Get(Form("hV0Asymmetry_V0M_%s",trigName.Data()));
  TH1D* hV0Asymmetry_TKL= (TH1D*)fin->Get(Form("hV0Asymmetry_TKL_%s",trigName.Data()));
  TH1D* hV0Decision_V0M= (TH1D*)fin->Get(Form("hV0Decision_V0M_%s",trigName.Data()));
  TH1D* hV0Decision_TKL= (TH1D*)fin->Get(Form("hV0Decision_TKL_%s",trigName.Data()));
  TH1D* hIncompleteEvents_V0M= (TH1D*)fin->Get(Form("hIncompleteEvents_V0M_%s",trigName.Data()));
  TH1D* hIncompleteEvents_TKL= (TH1D*)fin->Get(Form("hIncompleteEvents_TKL_%s",trigName.Data()));
  TH2D* hTrackClusterCorr=(TH2D*)fin->Get(Form("hTrackClusterCorr_%s",trigName.Data()));
  TH2D* hTrackClusterCorrCut=(TH2D*)fin->Get(Form("hTrackClusterCorrCut_%s",trigName.Data()));
  TH2D* hOnlineOfflineFastOR=(TH2D*)fin->Get(Form("hOnlineOfflineFastOR_%s",trigName.Data()));
  TH2D* hOnlineOfflineFastORCut=(TH2D*)fin->Get(Form("hOnlineOfflineFastORCut_%s",trigName.Data()));
  TH2D* hV0AsymmCorr=(TH2D*)fin->Get(Form("hV0AsymmCorr_%s",trigName.Data()));
  TH2D* hV0AsymmCorrCut=(TH2D*)fin->Get(Form("hV0AsymmCorrCut_%s",trigName.Data()));
  TH2F* h_V0M_TKL = (TH2F*)fin->Get(Form("h_V0M_TKL_%s",trigName.Data()));
  TH2F* hVz_V0M = (TH2F*)fin->Get(Form("hVz_V0M_%s",trigName.Data()));
  TH2F* hVz_TKL = (TH2F*)fin->Get(Form("hVz_TKL_%s",trigName.Data()));

  TH2F* hV0MTKL = (TH2F*)fin->Get(Form("hV0MTKL_%s",trigName.Data()));
  TH1D* hEventsRun = (TH1D*)fin->Get(Form("hEventsRun_%s",trigName.Data()));
  TH2F* hV0MMultRun = (TH2F*)fin->Get(Form("hV0MMultRun_%s",trigName.Data()));
  TH2F* hTKLMultRun = (TH2F*)fin->Get(Form("hTKLMultRun_%s",trigName.Data()));
  TH2F* hV0MCentRun = (TH2F*)fin->Get(Form("hV0MCentRun_%s",trigName.Data()));
  TH2F* hTKLCentRun = (TH2F*)fin->Get(Form("hTKLCentRun_%s",trigName.Data()));
  TH2F* hVzRun = (TH2F*)fin->Get(Form("hVzRun_%s",trigName.Data()));
  TH2F* hVzV0MCent = (TH2F*)fin->Get(Form("hVzV0MCent_%s",trigName.Data()));
  TH2F* hVzTKLCent = (TH2F*)fin->Get(Form("hVzTKLCent_%s",trigName.Data()));
  TH1D* hNeventsV0M = (TH1D*)fin->Get(Form("hNeventsV0M_%s",trigName.Data()));
  TH1D* hNeventsTKL = (TH1D*)fin->Get(Form("hNeventsTKL_%s",trigName.Data()));
  TH2D* hV0MNch = (TH2D*)fin->Get(Form("hV0MNch_%s",trigName.Data()));
  TH2D* hTKLNch = (TH2D*)fin->Get(Form("hTKLNch_%s",trigName.Data()));
  TH2F* hV0MCentPerc = (TH2F*)fin->Get(Form("hV0MCentPerc_%s",trigName.Data()));
  TH2F* hV0MCentPercLog = (TH2F*)fin->Get(Form("hV0MCentPercLog_%s",trigName.Data()));
  TH2F* hV0MMultMc = (TH2F*)fin->Get(Form("hV0MMultMc_%s",trigName.Data()));
  TH2F* hV0MCentMc = (TH2F*)fin->Get(Form("hV0MCentMc_%s",trigName.Data()));
  TH2F* hTKLCentMc = (TH2F*)fin->Get(Form("hTKLCentMc_%s",trigName.Data()));

  TH2D* hVertexSPDprim = (TH2D*)fin->Get(Form("hVertexSPDprim_%s",trigName.Data()));
  TH2D* hVertexSPDprimMult = (TH2D*)fin->Get(Form("hVertexSPDprimMult_%s",trigName.Data()));
  TH2D* hVertexResMult = (TH2D*)fin->Get(Form("hVertexResMult_%s",trigName.Data()));
  TH2D* hVertexContribMult = (TH2D*)fin->Get(Form("hVertexContribMult_%s",trigName.Data()));
  TH2D* hUncorr_V0M = (TH2D*)fin->Get(Form("hUncorr_V0M_%s",trigName.Data()));
  TH2D* hUncorr_TKL = (TH2D*)fin->Get(Form("hUncorr_TKL_%s",trigName.Data()));
  TH2D* hCorr_V0M = (TH2D*)fin->Get(Form("hCorr_V0M_%s",trigName.Data()));
  TH2D* hCorr_TKL = (TH2D*)fin->Get(Form("hCorr_TKL_%s",trigName.Data()));
  TH2D* hVzNch = (TH2D*)fin->Get(Form("hVzNch_%s",trigName.Data()));

  //*****************************************************************************
  // event statistics
  TCanvas *cEvent = new TCanvas(Form("cEvent_%s",trigName.Data()),"event statistics",700,500);
  cEvent->cd();
  TH1D* hEventCount1 = (TH1D*)hEventCount->ProjectionY("");
  hEventCount1->SetName(Form("%s_yproj",hEventCount->GetName()));
  hEventCount1->DrawCopy();
  cEvent->Update();
  if(hEventCount_CVHMV0M_B){
    TCanvas *cEvent_CVHMV0M_B = new TCanvas(Form("cEvent_Only_CVHMV0M_B"),"event statistics - HMV0",700,500);
    cEvent_CVHMV0M_B->cd();
    TH1D* hEventCount1_CVHMV0M_B = (TH1D*)hEventCount_CVHMV0M_B->ProjectionY();
    hEventCount1_CVHMV0M_B->SetName(Form("%s_yproj",hEventCount_CVHMV0M_B->GetName()));
    hEventCount1_CVHMV0M_B->DrawCopy("");
    cEvent_CVHMV0M_B->Update();
    hEventCount_CVHMV0M_B->Print("");
    hEventCount->Print("");
  }
  /*TCanvas *cEventStat = new TCanvas(Form("cEventStat_%s",trigName.Data()),"event statistics 2",700,500);
    cEventStat->cd();
    for(Int_t nn = 1; nn <= 12; nn++){
    hEventCount->GetYaxis()->SetRange(nn,nn);
    TH1D* hEventCount2 = (TH1D*)hEventCount->Project3D("x_NUF_NOF");
    hEventCount2->SetName(Form("%s_xproj_%i",hEventCount->GetName(),nn));
    hEventCount2->SetLineColor(nn);
    hEventCount2->DrawCopy(nn==1 ? "" : "same");
    }
    cEventStat->Update();*/

  //*****************************************************************************
  // events per run
  TCanvas *cRun = new TCanvas(Form("cRun_%s",trigName.Data()),"events per run",700,500);
  cRun->cd();
  hEventsRun->DrawCopy();
  cRun->Update();

  //*****************************************************************************
  // pileup

  TCanvas *cPileup=new TCanvas(Form("cPileup_%s",trigName.Data()),"cPileup",1200,1000);
  cPileup->Divide(2,2);
  cPileup->cd(1);
  hAllEvents_V0M->SetLineColor(1);
  hAllEvents_V0M->SetMinimum(kBlack);
  hAllEvents_V0M->GetXaxis()->SetRangeUser(0,600);
  hAllEvents_V0M->DrawCopy();
  hOutOfBunchPileup_V0M->SetLineColor(kRed);
  hOutOfBunchPileup_V0M->GetXaxis()->SetRangeUser(0,600);
  hOutOfBunchPileup_V0M->DrawCopy("same");
  hV0Asymmetry_V0M->SetLineColor(kGreen+2);
  hV0Asymmetry_V0M->GetXaxis()->SetRangeUser(0,600);
  hV0Asymmetry_V0M->DrawCopy("same");
  hPileupMV_V0M->SetLineColor(kOrange+2);
  hPileupMV_V0M->GetXaxis()->SetRangeUser(0,600);
  hPileupMV_V0M->DrawCopy("same");
  hPileupSPD_V0M->SetLineColor(kGreen+3);
  hPileupSPD_V0M->GetXaxis()->SetRangeUser(0,600);
  hPileupSPD_V0M->DrawCopy("same");
  hTrackClusterCut_V0M->SetLineColor(kCyan);
  hTrackClusterCut_V0M->GetXaxis()->SetRangeUser(0,600);
  hTrackClusterCut_V0M->DrawCopy("same");
  hV0Decision_V0M->SetLineColor(kBlue);
  hV0Decision_V0M->GetXaxis()->SetRangeUser(0,600);
  hV0Decision_V0M->DrawCopy("same");
  hIncompleteEvents_V0M->SetLineColor(kViolet+4);
  hIncompleteEvents_V0M->GetXaxis()->SetRangeUser(0,600);
  hIncompleteEvents_V0M->DrawCopy("same");
  gPad->SetLogy();
  gPad->SetGridy();
  //gPad->BuildLegend(0.1,0.7,0.9,0.9)->SetFillStyle(0);

  cPileup->cd(2);
  hAllEvents_TKL->SetLineColor(1);
  hAllEvents_TKL->SetMinimum(kBlack);
  hAllEvents_TKL->GetXaxis()->SetRangeUser(0,600);
  hAllEvents_TKL->DrawCopy();
  hOutOfBunchPileup_TKL->SetLineColor(kRed);
  hOutOfBunchPileup_TKL->GetXaxis()->SetRangeUser(0,600);
  hOutOfBunchPileup_TKL->DrawCopy("same");
  hV0Asymmetry_TKL->SetLineColor(kGreen+2);
  hV0Asymmetry_TKL->GetXaxis()->SetRangeUser(0,600);
  hV0Asymmetry_TKL->DrawCopy("same");
  hPileupMV_TKL->SetLineColor(kOrange+2);
  hPileupMV_TKL->GetXaxis()->SetRangeUser(0,600);
  hPileupMV_TKL->DrawCopy("same");
  hPileupSPD_TKL->SetLineColor(kGreen+3);
  hPileupSPD_TKL->GetXaxis()->SetRangeUser(0,600);
  hPileupSPD_TKL->DrawCopy("same");
  hTrackClusterCut_TKL->SetLineColor(kCyan);
  hTrackClusterCut_TKL->GetXaxis()->SetRangeUser(0,600);
  hTrackClusterCut_TKL->DrawCopy("same");
  hV0Decision_TKL->SetLineColor(kBlue);
  hV0Decision_TKL->GetXaxis()->SetRangeUser(0,600);
  hV0Decision_TKL->DrawCopy("same");
  hIncompleteEvents_TKL->SetLineColor(kViolet+4);
  hIncompleteEvents_TKL->GetXaxis()->SetRangeUser(0,600);
  hIncompleteEvents_TKL->DrawCopy("same");
  gPad->SetLogy();
  gPad->SetGridy();

  //  if(hAllEvents_TklMult_dphi_V0M){
  //    //  diffractive events - tkl
  //    for(Int_t ibindphi=1;ibindphi<= hAllEvents_TklMult_dphi_V0M->GetYaxis()->GetNbins();ibindphi++){
  //      //project every bin to see the mult in dphi<maxdphi
  //      hAllEvents_TklMult_dphi_V0M->GetYaxis()->SetRange(ibindphi,ibindphi);
  //      hAllEvents_TklMult_dphi_TKL->GetYaxis()->SetRange(ibindphi,ibindphi);
  //      hSingleDiffractive_TklMult_dphi_V0M->GetYaxis()->SetRange(ibindphi,ibindphi);
  //      hDoubleDiffractive_TklMult_dphi_V0M->GetYaxis()->SetRange(ibindphi,ibindphi);
  //      hSingleDiffractive_TklMult_dphi_TKL->GetYaxis()->SetRange(ibindphi,ibindphi);
  //      hDoubleDiffractive_TklMult_dphi_TKL->GetYaxis()->SetRange(ibindphi,ibindphi);
  //      Double_t maxdphi=hAllEvents_TklMult_dphi_V0M->GetYaxis()->GetBinUpEdge(ibindphi);
  //      TH2D* h2DAllEvents_TklMult_dphi_V0M=(TH2D*)hAllEvents_TklMult_dphi_V0M->Project3D("xz");
  //      TH2D* h2DAllEvents_TklMult_dphi_TKL=(TH2D*)hAllEvents_TklMult_dphi_TKL->Project3D("xz");
  //      TH2D* h2DSingleDiffractive_TklMult_dphi_V0M=(TH2D*)hSingleDiffractive_TklMult_dphi_V0M->Project3D("xz");
  //      TH2D* h2DSingleDiffractive_TklMult_dphi_TKL=(TH2D*)hSingleDiffractive_TklMult_dphi_TKL->Project3D("xz");
  //      TH2D* h2DDoubleDiffractive_TklMult_dphi_V0M=(TH2D*)hDoubleDiffractive_TklMult_dphi_V0M->Project3D("xz");
  //      TH2D* h2DDoubleDiffractive_TklMult_dphi_TKL=(TH2D*)hDoubleDiffractive_TklMult_dphi_TKL->Project3D("xz");
  //      //calculate integral from m=im to max to simulate the cut in multiplicity
  //      for(Int_t ibinx=1;ibinx<=h2DAllEvents_TklMult_dphi_V0M->GetXaxis()->GetNbins();ibinx++){//x = multiplicity class (V0M or TKL)
  //        for(Int_t ibiny=1;ibiny<=h2DAllEvents_TklMult_dphi_V0M->GetYaxis()->GetNbins();ibiny++){//y= multiplicity of the event
  //          Int_t nbinsy=h2DAllEvents_TklMult_dphi_V0M->GetYaxis()->GetNbins();
  //          h2DAllEvents_TklMult_dphi_V0M->SetBinContent(ibinx,ibiny,h2DAllEvents_TklMult_dphi_V0M->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DAllEvents_TklMult_dphi_TKL->SetBinContent(ibinx,ibiny,h2DAllEvents_TklMult_dphi_V0M->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DSingleDiffractive_TklMult_dphi_V0M->SetBinContent(ibinx,ibiny,h2DSingleDiffractive_TklMult_dphi_V0M->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DSingleDiffractive_TklMult_dphi_TKL->SetBinContent(ibinx,ibiny,h2DSingleDiffractive_TklMult_dphi_TKL->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DDoubleDiffractive_TklMult_dphi_V0M->SetBinContent(ibinx,ibiny,h2DDoubleDiffractive_TklMult_dphi_TKL->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DDoubleDiffractive_TklMult_dphi_TKL->SetBinContent(ibinx,ibiny,h2DDoubleDiffractive_TklMult_dphi_TKL->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //        }
  //      }
  //      h2DSingleDiffractive_TklMult_dphi_V0M->Divide(h2DAllEvents_TklMult_dphi_V0M);
  //      h2DDoubleDiffractive_TklMult_dphi_V0M->Divide(h2DAllEvents_TklMult_dphi_V0M);
  //      h2DSingleDiffractive_TklMult_dphi_TKL->Divide(h2DAllEvents_TklMult_dphi_TKL);
  //      h2DDoubleDiffractive_TklMult_dphi_TKL->Divide(h2DAllEvents_TklMult_dphi_TKL);
  //
  //      h2DAllEvents_TklMult_dphi_V0M->SetTitle(Form("All Events (dphi<%.1f)",maxdphi));
  //      h2DAllEvents_TklMult_dphi_TKL->SetTitle(Form("All Events (dphi<%.1f)",maxdphi));
  //      h2DSingleDiffractive_TklMult_dphi_V0M->SetTitle(Form("Single diffractive (dphi<%.1f)",maxdphi));
  //      h2DSingleDiffractive_TklMult_dphi_TKL->SetTitle(Form("Single diffractive (dphi<%.1f)",maxdphi));
  //      h2DDoubleDiffractive_TklMult_dphi_V0M->SetTitle(Form("Double diffractive (dphi<%.1f)",maxdphi));
  //      h2DDoubleDiffractive_TklMult_dphi_TKL->SetTitle(Form("Double diffractive (dphi<%.1f)",maxdphi));
  //
  //      //normalize all events to the first bin to see the rejection factor
  //      for(Int_t ibinx=1;ibinx<=h2DAllEvents_TklMult_dphi_V0M->GetXaxis()->GetNbins();ibinx++){
  //        for(Int_t ibiny=2;ibiny<=h2DAllEvents_TklMult_dphi_V0M->GetYaxis()->GetNbins();ibiny++){
  //          h2DAllEvents_TklMult_dphi_V0M->SetBinContent(ibinx,ibiny,h2DAllEvents_TklMult_dphi_V0M->GetBinContent(ibinx,ibiny)/h2DAllEvents_TklMult_dphi_V0M->GetBinContent(ibinx,1));
  //          h2DAllEvents_TklMult_dphi_TKL->SetBinContent(ibinx,ibiny,h2DAllEvents_TklMult_dphi_TKL->GetBinContent(ibinx,ibiny)/h2DAllEvents_TklMult_dphi_TKL->GetBinContent(ibinx,1));
  //        }
  //        h2DAllEvents_TklMult_dphi_V0M->SetBinContent(ibinx,1,1.);
  //        h2DAllEvents_TklMult_dphi_TKL->SetBinContent(ibinx,1,1.);
  //      }
  //      TCanvas *cDiffractionFraction=new TCanvas(Form("cDiffractionFraction_%s_dphi%d",trigName.Data(),ibindphi),Form("cDiffractionFraction_%s_dphi%d",trigName.Data(),ibindphi),1200,1400);
  //      h2DAllEvents_TklMult_dphi_V0M->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DAllEvents_TklMult_dphi_TKL->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DSingleDiffractive_TklMult_dphi_V0M->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DSingleDiffractive_TklMult_dphi_TKL->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DDoubleDiffractive_TklMult_dphi_V0M->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DDoubleDiffractive_TklMult_dphi_TKL->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DAllEvents_TklMult_dphi_V0M->GetXaxis()->SetRangeUser(0,2);
  //      h2DAllEvents_TklMult_dphi_TKL->GetXaxis()->SetRangeUser(0,2);
  //      h2DSingleDiffractive_TklMult_dphi_V0M->GetXaxis()->SetRangeUser(0,2);
  //      h2DSingleDiffractive_TklMult_dphi_TKL->GetXaxis()->SetRangeUser(0,2);
  //      h2DDoubleDiffractive_TklMult_dphi_V0M->GetXaxis()->SetRangeUser(0,2);
  //      h2DDoubleDiffractive_TklMult_dphi_TKL->GetXaxis()->SetRangeUser(0,2);
  //      cDiffractionFraction->Divide(2,3);
  //      cDiffractionFraction->cd(1);
  //      h2DAllEvents_TklMult_dphi_V0M->DrawCopy("colz");
  //      cDiffractionFraction->cd(2);
  //      h2DAllEvents_TklMult_dphi_TKL->DrawCopy("colz");
  //      cDiffractionFraction->cd(3);
  //      h2DSingleDiffractive_TklMult_dphi_V0M->DrawCopy("colz");
  //      cDiffractionFraction->cd(4);
  //      h2DSingleDiffractive_TklMult_dphi_TKL->DrawCopy("colz");
  //      cDiffractionFraction->cd(5);
  //      h2DDoubleDiffractive_TklMult_dphi_V0M->DrawCopy("colz");
  //      cDiffractionFraction->cd(6);
  //      h2DDoubleDiffractive_TklMult_dphi_TKL->DrawCopy("colz");
  //    }
  //
  //    //  diffractive events - trk
  //    for(Int_t ibinpt=1;ibinpt<= hAllEvents_TrkMult_pt_V0M->GetYaxis()->GetNbins();ibinpt++){
  //      //project every bin to see the mult in pt>minpt
  //      hAllEvents_TrkMult_pt_V0M->GetYaxis()->SetRange(ibinpt,ibinpt);
  //      hAllEvents_TrkMult_pt_TKL->GetYaxis()->SetRange(ibinpt,ibinpt);
  //      hSingleDiffractive_TrkMult_pt_V0M->GetYaxis()->SetRange(ibinpt,ibinpt);
  //      hDoubleDiffractive_TrkMult_pt_V0M->GetYaxis()->SetRange(ibinpt,ibinpt);
  //      hSingleDiffractive_TrkMult_pt_TKL->GetYaxis()->SetRange(ibinpt,ibinpt);
  //      hDoubleDiffractive_TrkMult_pt_TKL->GetYaxis()->SetRange(ibinpt,ibinpt);
  //      Double_t minpt=hAllEvents_TrkMult_pt_V0M->GetYaxis()->GetBinUpEdge(ibinpt);
  //      TH2D* h2DAllEvents_TrkMult_pt_V0M=(TH2D*)hAllEvents_TrkMult_pt_V0M->Project3D("xz");
  //      TH2D* h2DAllEvents_TrkMult_pt_TKL=(TH2D*)hAllEvents_TrkMult_pt_TKL->Project3D("xz");
  //      TH2D* h2DSingleDiffractive_TrkMult_pt_V0M=(TH2D*)hSingleDiffractive_TrkMult_pt_V0M->Project3D("xz");
  //      TH2D* h2DSingleDiffractive_TrkMult_pt_TKL=(TH2D*)hSingleDiffractive_TrkMult_pt_TKL->Project3D("xz");
  //      TH2D* h2DDoubleDiffractive_TrkMult_pt_V0M=(TH2D*)hDoubleDiffractive_TrkMult_pt_V0M->Project3D("xz");
  //      TH2D* h2DDoubleDiffractive_TrkMult_pt_TKL=(TH2D*)hDoubleDiffractive_TrkMult_pt_TKL->Project3D("xz");
  //      //calculate integral from m=im to max to simulate the cut in multiplicity
  //      for(Int_t ibinx=1;ibinx<=h2DAllEvents_TrkMult_pt_V0M->GetXaxis()->GetNbins();ibinx++){//x = multiplicity class (V0M or TKL)
  //        for(Int_t ibiny=1;ibiny<=h2DAllEvents_TrkMult_pt_V0M->GetYaxis()->GetNbins();ibiny++){//y= multiplicity of the event
  //          Int_t nbinsy=h2DAllEvents_TrkMult_pt_V0M->GetYaxis()->GetNbins();
  //          h2DAllEvents_TrkMult_pt_V0M->SetBinContent(ibinx,ibiny,h2DAllEvents_TrkMult_pt_V0M->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DAllEvents_TrkMult_pt_TKL->SetBinContent(ibinx,ibiny,h2DAllEvents_TrkMult_pt_V0M->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DSingleDiffractive_TrkMult_pt_V0M->SetBinContent(ibinx,ibiny,h2DSingleDiffractive_TrkMult_pt_V0M->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DSingleDiffractive_TrkMult_pt_TKL->SetBinContent(ibinx,ibiny,h2DSingleDiffractive_TrkMult_pt_TKL->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DDoubleDiffractive_TrkMult_pt_V0M->SetBinContent(ibinx,ibiny,h2DDoubleDiffractive_TrkMult_pt_TKL->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //          h2DDoubleDiffractive_TrkMult_pt_TKL->SetBinContent(ibinx,ibiny,h2DDoubleDiffractive_TrkMult_pt_TKL->Integral(ibinx,ibinx,ibiny,nbinsy+1));
  //        }
  //      }
  //      h2DSingleDiffractive_TrkMult_pt_V0M->Divide(h2DAllEvents_TrkMult_pt_V0M);
  //      h2DDoubleDiffractive_TrkMult_pt_V0M->Divide(h2DAllEvents_TrkMult_pt_V0M);
  //      h2DSingleDiffractive_TrkMult_pt_TKL->Divide(h2DAllEvents_TrkMult_pt_TKL);
  //      h2DDoubleDiffractive_TrkMult_pt_TKL->Divide(h2DAllEvents_TrkMult_pt_TKL);
  //
  //      h2DAllEvents_TrkMult_pt_V0M->SetTitle(Form("All Events (pt>%.1f)",minpt));
  //      h2DAllEvents_TrkMult_pt_TKL->SetTitle(Form("All Events (pt>%.1f)",minpt));
  //      h2DSingleDiffractive_TrkMult_pt_V0M->SetTitle(Form("Single diffractive (pt>%.1f)",minpt));
  //      h2DSingleDiffractive_TrkMult_pt_TKL->SetTitle(Form("Single diffractive (pt>%.1f)",minpt));
  //      h2DDoubleDiffractive_TrkMult_pt_V0M->SetTitle(Form("Double diffractive (pt>%.1f)",minpt));
  //      h2DDoubleDiffractive_TrkMult_pt_TKL->SetTitle(Form("Double diffractive (pt>%.1f)",minpt));
  //
  //      //normalize all events to the first bin to see the rejection factor
  //      for(Int_t ibinx=1;ibinx<=h2DAllEvents_TrkMult_pt_V0M->GetXaxis()->GetNbins();ibinx++){
  //        for(Int_t ibiny=2;ibiny<=h2DAllEvents_TrkMult_pt_V0M->GetYaxis()->GetNbins();ibiny++){
  //          h2DAllEvents_TrkMult_pt_V0M->SetBinContent(ibinx,ibiny,h2DAllEvents_TrkMult_pt_V0M->GetBinContent(ibinx,ibiny)/h2DAllEvents_TrkMult_pt_V0M->GetBinContent(ibinx,1));
  //          h2DAllEvents_TrkMult_pt_TKL->SetBinContent(ibinx,ibiny,h2DAllEvents_TrkMult_pt_TKL->GetBinContent(ibinx,ibiny)/h2DAllEvents_TrkMult_pt_TKL->GetBinContent(ibinx,1));
  //        }
  //        h2DAllEvents_TrkMult_pt_V0M->SetBinContent(ibinx,1,1.);
  //        h2DAllEvents_TrkMult_pt_TKL->SetBinContent(ibinx,1,1.);
  //      }
  //      TCanvas *cDiffractionFraction=new TCanvas(Form("cDiffractionFraction_%s_pt%d",trigName.Data(),ibinpt),Form("cDiffractionFraction_%s_pt%d",trigName.Data(),ibinpt),1200,1400);
  //      h2DAllEvents_TrkMult_pt_V0M->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DAllEvents_TrkMult_pt_TKL->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DSingleDiffractive_TrkMult_pt_V0M->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DSingleDiffractive_TrkMult_pt_TKL->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DDoubleDiffractive_TrkMult_pt_V0M->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DDoubleDiffractive_TrkMult_pt_TKL->GetYaxis()->SetRangeUser(0,19.99);
  //      h2DAllEvents_TrkMult_pt_V0M->GetXaxis()->SetRangeUser(0,2);
  //      h2DAllEvents_TrkMult_pt_TKL->GetXaxis()->SetRangeUser(0,2);
  //      h2DSingleDiffractive_TrkMult_pt_V0M->GetXaxis()->SetRangeUser(0,2);
  //      h2DSingleDiffractive_TrkMult_pt_TKL->GetXaxis()->SetRangeUser(0,2);
  //      h2DDoubleDiffractive_TrkMult_pt_V0M->GetXaxis()->SetRangeUser(0,2);
  //      h2DDoubleDiffractive_TrkMult_pt_TKL->GetXaxis()->SetRangeUser(0,2);
  //      cDiffractionFraction->Divide(2,3);
  //      cDiffractionFraction->cd(1);
  //      h2DAllEvents_TrkMult_pt_V0M->DrawCopy("colz");
  //      cDiffractionFraction->cd(2);
  //      h2DAllEvents_TrkMult_pt_TKL->DrawCopy("colz");
  //      cDiffractionFraction->cd(3);
  //      h2DSingleDiffractive_TrkMult_pt_V0M->DrawCopy("colz");
  //      cDiffractionFraction->cd(4);
  //      h2DSingleDiffractive_TrkMult_pt_TKL->DrawCopy("colz");
  //      cDiffractionFraction->cd(5);
  //      h2DDoubleDiffractive_TrkMult_pt_V0M->DrawCopy("colz");
  //      cDiffractionFraction->cd(6);
  //      h2DDoubleDiffractive_TrkMult_pt_TKL->DrawCopy("colz");
  //    }
  //  }
  //V0asymmetry cut check
  TCanvas *cV0AsymmCorr=new TCanvas(Form("cV0AsymmCorr_%s",trigName.Data()),"cV0AsymmCorr",1200,800);
  gPad->SetRightMargin(0.15);
  cV0AsymmCorr->Divide(2,2);
  cV0AsymmCorr->cd(1);
  hV0AsymmCorr->DrawCopy("colz");
  gPad->SetLogz();
  cV0AsymmCorr->cd(2);
  hV0AsymmCorrCut->DrawCopy("colz");
  gPad->SetLogz();
  cV0AsymmCorr->cd(3);
  hAllEvents_V0M->DrawCopy();
  hV0Asymmetry_V0M->DrawCopy("same");
  gPad->SetLogy();
  gPad->BuildLegend(0.6,0.6,0.9,0.9);
  cV0AsymmCorr->cd(4);
  hAllEvents_V0M->DrawCopy();
  TH1F *hAllEvents_V0M_AfterAsymmCut=(TH1F*)hAllEvents_V0M->Clone("hAllEvents_V0M_AfterAsymmCut");
  hAllEvents_V0M_AfterAsymmCut->Add(hV0Asymmetry_V0M,-1);
  hAllEvents_V0M_AfterAsymmCut->DrawCopy("same");
  gPad->SetLogy();
  gPad->BuildLegend(0.6,0.6,0.9,0.9);
  cV0AsymmCorr->Update();

  //calculate pileup fractions
  hPileupSPD_V0M->Divide(hAllEvents_V0M);
  hPileupSPD_TKL->Divide(hAllEvents_TKL);
  hPileupMV_V0M->Divide(hAllEvents_V0M);
  hPileupMV_TKL->Divide(hAllEvents_TKL);
  hTrackClusterCut_V0M->Divide(hAllEvents_V0M);
  hTrackClusterCut_TKL->Divide(hAllEvents_TKL);
  hOutOfBunchPileup_V0M->Divide(hAllEvents_V0M);
  hOutOfBunchPileup_TKL->Divide(hAllEvents_TKL);
  hV0Asymmetry_V0M->Divide(hAllEvents_V0M);
  hV0Asymmetry_TKL->Divide(hAllEvents_TKL);
  hV0Decision_V0M->Divide(hAllEvents_V0M);
  hV0Decision_TKL->Divide(hAllEvents_TKL);
  hIncompleteEvents_V0M->Divide(hAllEvents_V0M);
  hIncompleteEvents_TKL->Divide(hAllEvents_TKL);

  cPileup->cd(3);
  hPileupSPD_V0M->SetMinimum(1e-5);
  hPileupSPD_V0M->SetMaximum(10);
  hPileupSPD_V0M->DrawCopy("");
  hPileupMV_V0M->DrawCopy("same");
  hTrackClusterCut_V0M->DrawCopy("same");
  hOutOfBunchPileup_V0M->DrawCopy("same");
  hV0Asymmetry_V0M->DrawCopy("same");
  hIncompleteEvents_V0M->DrawCopy("same");
  hV0Decision_V0M->DrawCopy("same");
  gPad->SetLogy();
  gPad->SetGridy();

  cPileup->cd(4);
  hPileupSPD_TKL->SetMinimum(1e-5);
  hPileupSPD_TKL->SetMaximum(10);
  hPileupSPD_TKL->DrawCopy("");
  hPileupMV_TKL->DrawCopy("same");
  hTrackClusterCut_TKL->DrawCopy("same");
  hOutOfBunchPileup_TKL->DrawCopy("same");
  hV0Asymmetry_TKL->DrawCopy("same");
  hIncompleteEvents_TKL->DrawCopy("same");
  hV0Decision_TKL->DrawCopy("same");
  gPad->SetLogy();
  gPad->SetGridy();
  cPileup->Update();

  TCanvas *cTrackClusterCorr=new TCanvas(Form("cTrackClusterCorr_%s",trigName.Data()),"cTrackClusterCorr",1200,800);
  gPad->SetRightMargin(0.15);
  cTrackClusterCorr->Divide(2,1);
  cTrackClusterCorr->cd(1);
  hTrackClusterCorr->DrawCopy("colz");
  gPad->SetLogz();
  cTrackClusterCorr->cd(2);
  hTrackClusterCorrCut->DrawCopy("colz");
  gPad->SetLogz();
  cTrackClusterCorr->Update();

  TCanvas *cOnlineOfflineFastOR=new TCanvas(Form("cOnlineOfflineFastOR_%s",trigName.Data()),"cOnlineOfflineFastOR",1200,800);
  gPad->SetRightMargin(0.15);
  cOnlineOfflineFastOR->Divide(2,1);
  cOnlineOfflineFastOR->cd(1);
  hOnlineOfflineFastOR->DrawCopy("colz");
  gPad->SetLogz();
  cOnlineOfflineFastOR->cd(2);
  hOnlineOfflineFastORCut->DrawCopy("colz");
  gPad->SetLogz();
  cOnlineOfflineFastOR->Update();

  TCanvas *c_V0M_TKL=new TCanvas(Form("c_V0M_TKL_%s",trigName.Data()),"c_V0M_TKL",1200,800);
  gPad->SetRightMargin(0.15);
  gPad->SetLogz();
  h_V0M_TKL->DrawCopy("colz");

  TCanvas *cVz=new TCanvas(Form("cVz_%s",trigName.Data()),"cVz",1200,800);
  cVz->Divide(2,1);
  cVz->cd(1);
  gPad->SetLogz();
  gPad->SetRightMargin(0.15);
  hVz_V0M->DrawCopy("colz");
  cVz->cd(2);
  gPad->SetLogz();
  gPad->SetRightMargin(0.15);
  hVz_TKL->DrawCopy("colz");

  //***************************************************************************
  // multiplicity calibration
  //Int_t colors[12] = {kViolet+2,kBlue,kCyan,kGreen+3,kYellow,kRed,kViolet+2,kBlue,kCyan,kGreen+3,kYellow,kRed};
  Int_t colors[13] = {kRed,kOrange+7,kYellow,kGreen+3,kBlue,kViolet+2,kRed,kOrange+7,kYellow,kGreen+3,kBlue,kViolet+2};

  TCanvas *cMeanMult3 = new TCanvas(Form("cMeanMult3_%s",trigName.Data()),"cMeanMult3",1600,500);
  cMeanMult3->Divide(2,1);
  cMeanMult3->cd(1);
  TH1F* hproj_corr_V0M2 = (TH1F*)hCorr_V0M->ProjectionY(Form("%s_proj",hCorr_V0M->GetName()));
  hproj_corr_V0M2->Scale(1./hproj_corr_V0M2->Integral());
  hproj_corr_V0M2->GetXaxis()->SetTitle("M/<M>");
  hproj_corr_V0M2->DrawCopy();
  gPad->SetLogy();

  for(Int_t ii = 1; ii < 13; ii++)
  {hproj_corr_V0M2->GetXaxis()->SetRangeUser(binscmin[ii]+0.0001,binscmax[ii]-0.0001); hproj_corr_V0M2->SetFillColor(colors[ii-1]); hproj_corr_V0M2->DrawCopy("same");}
  cMeanMult3->cd(2);
  TH1F* hproj_corr_TKL2 = (TH1F*)hCorr_TKL->ProjectionY(Form("%s_proj",hCorr_TKL->GetName()));
  hproj_corr_TKL2->Scale(1./hproj_corr_TKL2->Integral());
  hproj_corr_TKL2->DrawCopy();
  gPad->SetLogy();

  for(Int_t ii = 1; ii < 13; ii++)
  {hproj_corr_TKL2->GetXaxis()->SetRangeUser(binscmin[ii]+0.0001,binscmax[ii]-0.0001); hproj_corr_TKL2->SetFillColor(colors[ii-1]); hproj_corr_TKL2->DrawCopy("same");}
  cMeanMult3->Update();

  TCanvas *cMeanMult1 = new TCanvas(Form("cMeanMult1_%s",trigName.Data()),"cMeanMult1",1100,1000);
  cMeanMult1->Divide(2,2);
  cMeanMult1->cd(1);
  TProfile *tp1 = (TProfile*)hUncorr_V0M->ProfileX();
  hUncorr_V0M->GetYaxis()->SetRangeUser(0,200);
  hUncorr_V0M->DrawCopy("colz");
  tp1->SetLineColor(2); tp1->SetLineWidth(2);
  tp1->DrawCopy("same");
  cMeanMult1->cd(2);
  TProfile *tp2 = (TProfile*)hUncorr_TKL->ProfileX();
  hUncorr_TKL->GetYaxis()->SetRangeUser(0,40);
  hUncorr_TKL->DrawCopy("colz");
  tp2->SetLineColor(2); tp2->SetLineWidth(2);
  tp2->DrawCopy("same");
  cMeanMult1->cd(3);
  TProfile *tp3 = (TProfile*)hCorr_V0M->ProfileX();
  hCorr_V0M->GetYaxis()->SetRangeUser(0,2.5);
  hCorr_V0M->DrawCopy("colz");
  tp3->SetLineColor(2); tp3->SetLineWidth(2);
  tp3->DrawCopy("same");
  cMeanMult1->cd(4);
  TProfile *tp4 = (TProfile*)hCorr_TKL->ProfileX();
  hCorr_TKL->GetYaxis()->SetRangeUser(0,2.5);
  hCorr_TKL->DrawCopy("colz");
  tp4->SetLineColor(2); tp4->SetLineWidth(2);
  tp4->DrawCopy("same");
  cMeanMult1->Update();

  TCanvas *cMeanMult2 = new TCanvas(Form("cMeanMult2_%s",trigName.Data()),"cMeanMult2",1600,1000);
  cMeanMult2->Divide(2,3);
  TH1F* hproj_corr_V0M_full = (TH1F*)hCorr_V0M->ProjectionY(Form("%s_proj_full",hCorr_V0M->GetName()));
  hproj_corr_V0M_full->Scale(Double_t(hproj_corr_V0M_full->FindBin(2.5))/hproj_corr_V0M_full->Integral());
  TH1F* hproj_corr_TKL_full = (TH1F*)hCorr_TKL->ProjectionY(Form("%s_proj_full",hCorr_TKL->GetName()));
  hproj_corr_TKL_full->Scale(Double_t(hproj_corr_TKL_full->FindBin(2.5))/hproj_corr_TKL_full->Integral());
  for(Int_t i = 1; i <= hUncorr_V0M->GetXaxis()->GetNbins(); i++)
  {
    TH1F* hproj_uncorr_V0M = (TH1F*)hUncorr_V0M->ProjectionY(Form("%s_proj_%i",hUncorr_V0M->GetName(),i),i,i);
    if(hproj_uncorr_V0M->Integral() > 0)
    {
      cMeanMult2->cd(1);
      hproj_uncorr_V0M->Scale(Double_t(hproj_uncorr_V0M->FindBin(200))/hproj_uncorr_V0M->Integral());
      hproj_uncorr_V0M->SetLineColor(i%9+1);
      hproj_uncorr_V0M->SetMaximum(3.);
      hproj_uncorr_V0M->DrawCopy(i==1 ? "" : "same");
    }

    TH1F* hproj_uncorr_TKL = (TH1F*)hUncorr_TKL->ProjectionY(Form("%s_proj_%i",hUncorr_TKL->GetName(),i),i,i);
    if(hproj_uncorr_TKL->Integral() > 0)
    {
      cMeanMult2->cd(2);
      hproj_uncorr_TKL->Scale(Double_t(hproj_uncorr_TKL->FindBin(40))/hproj_uncorr_TKL->Integral());
      hproj_uncorr_TKL->SetLineColor(i%9+1);
      hproj_uncorr_TKL->SetMaximum(3.);
      hproj_uncorr_TKL->DrawCopy(i==1 ? "" : "same");
    }

    TH1F* hproj_corr_V0M = (TH1F*)hCorr_V0M->ProjectionY(Form("%s_proj_%i",hCorr_V0M->GetName(),i),i,i);
    if(hproj_corr_V0M->Integral() > 0)
    {
      cMeanMult2->cd(3);
      hproj_corr_V0M->Scale(Double_t(hproj_corr_V0M->FindBin(2.5))/hproj_corr_V0M->Integral());
      hproj_corr_V0M->SetLineColor(i%9+1);
      hproj_corr_V0M->SetMaximum(3.);
      hproj_corr_V0M->DrawCopy(i==1 ? "" : "same");
      cMeanMult2->cd(5);
      hproj_corr_V0M->Divide(hproj_corr_V0M_full);
      //      hproj_corr_V0M->Rebin(5);
      //      hproj_corr_V0M->Scale(1./5.);
      hproj_corr_V0M->SetMaximum(1.5); hproj_corr_V0M->SetMinimum(0.5);
      hproj_corr_V0M->DrawCopy(i==1 ? "" : "same");
    }

    TH1F* hproj_corr_TKL = (TH1F*)hCorr_TKL->ProjectionY(Form("%s_proj_%i",hCorr_TKL->GetName(),i),i,i);
    if(hproj_corr_TKL->Integral() > 0)
    {
      cMeanMult2->cd(4);
      hproj_corr_TKL->Scale(Double_t(hproj_corr_TKL->FindBin(2.5))/hproj_corr_TKL->Integral());
      hproj_corr_TKL->SetLineColor(i%9+1);
      hproj_corr_TKL->SetMaximum(3.);
      hproj_corr_TKL->DrawCopy(i==1 ? "" : "same");
      cMeanMult2->cd(6);
      hproj_corr_TKL->Divide(hproj_corr_TKL_full);
      //hproj_corr_TKL->Rebin(5);
      //hproj_corr_TKL->Scale(1./5.);
      hproj_corr_TKL->SetMaximum(1.5); hproj_corr_TKL->SetMinimum(0.5);
      hproj_corr_TKL->DrawCopy(i==1 ? "" : "same");
    }
  }
  cMeanMult2->Update();

  //*****************************************************************************
  // correlations between percentiles
  TCanvas *cV0MTKL = new TCanvas(Form("cV0MTKL_%s",trigName.Data()),"V0M vs TKL",1400,1000);
  cV0MTKL->Divide(1,2);
  cV0MTKL->cd(1);
  hV0MTKL->DrawCopy("colz");
  gPad->SetLogz();
  cV0MTKL->cd(2);
  TLegend *tleg = new TLegend(0.4,0.4,0.7,0.9,"TKL M/<M> bins");
  for(Int_t i = 0; i < 7; i++)
  {
    TH1F* htemp = (TH1F*)hV0MTKL->ProjectionX(Form("%s_proj_%i",hV0MTKL->GetName(),i),5*i+1,5*(i+1));
    htemp->Scale(1./htemp->Integral());
    htemp->SetTitle(""); htemp->GetYaxis()->SetTitle("N_{events}     ");
    htemp->GetYaxis()->SetTitleOffset(1.4);
    htemp->SetLineColor(i+1);
    htemp->SetLineWidth(2);
    if(i==9) htemp->SetLineColor(kViolet);
    htemp->SetMinimum(0);
    htemp->DrawCopy(i==0 ? "" : "same");
    tleg->AddEntry(htemp,Form("%i-%i",i,i+1),"L");
  }
  tleg->Draw();
  cV0MTKL->Update();

  TCanvas *cCorrPerc = new TCanvas(Form("cCorrPerc_%s",trigName.Data()),"correlations between multiplicity estimators",1400,1000);
  gPad->SetLogz();
  hV0MTKL->DrawCopy("logzcolz");
  cCorrPerc->Update();

  //*****************************************************************************
  // vertex QA
  TCanvas *cVzRun = new TCanvas(Form("cVzRun_%s",trigName.Data()),"vz vs run number",800,500);
  cVzRun->cd();
  hVzRun->DrawCopy("colz");
  TProfile *hVzRun_prof = (TProfile*)hVzRun->ProfileX();
  hVzRun_prof->SetLineColor(2);
  hVzRun_prof->SetLineWidth(2);
  hVzRun_prof->DrawCopy("same");
  gPad->SetLogz();
  cVzRun->Update();

  TCanvas *cVzCent = new TCanvas(Form("cVzCent_%s",trigName.Data()),"vz vs multiplicity estimators",1600,1000);
  cVzCent->Divide(2,2);
  for(Int_t i = 0; i < 7 /*hVzTKLCent->GetYaxis()->GetNbins()/2*/; i++)
  {
    // V0M
    cVzCent->cd(1);
    hVzV0MCent->GetYaxis()->SetRange(2*i+1,2*i+2);
    TH1F* hVzV0M = (TH1F*)hVzV0MCent->ProjectionX();
    hVzV0M->Scale(1./hVzV0M->Integral());
    hVzV0M->SetLineColor(i+1);
    if(i==9) hVzV0M->SetLineColor(kViolet);
    hVzV0M->DrawCopy(i==0 ? "" : "same");
    // TKL
    cVzCent->cd(2);
    hVzTKLCent->GetYaxis()->SetRange(2*i+1,2*i+2);
    TH1F* hVzTKL = (TH1F*)hVzTKLCent->ProjectionX();
    hVzTKL->Scale(1./hVzTKL->Integral());
    hVzTKL->SetLineColor(i+1);
    if(i==9) hVzTKL->SetLineColor(kViolet);
    hVzTKL->DrawCopy(i==0 ? "" : "same");
  }
  cVzCent->Update();

  //*****************************************************************************
  // stability of multiplicity classes as a function of run number
  TCanvas *cCentRun = new TCanvas(Form("cCentRun_%s",trigName.Data()),"multiplicity classes vs run number",1600,1000);
  cCentRun->Divide(2,2);
  for(Int_t i = 0; i < 7; i++)
  {
    // V0M
    cCentRun->cd(1);
    hV0MCentRun->GetYaxis()->SetRange(i*10+1,(i+1)*10);
    TH1F* hV0MCentRunProj = (TH1F*)hV0MCentRun->ProjectionX();
    hV0MCentRunProj->Divide(hEventsRun);
    hV0MCentRunProj->Scale(hV0MCentRunProj->GetXaxis()->GetNbins()/hV0MCentRunProj->Integral()); // put them all on the same scale
    hV0MCentRunProj->SetLineColor(i== 9 ? kViolet : i+1);
    hV0MCentRunProj->SetMinimum(0.5); hV0MCentRunProj->SetMaximum(1.5);
    if(i>4){hV0MCentRunProj->Rebin(2); hV0MCentRunProj->Scale(0.5);}
    hV0MCentRunProj->DrawCopy(i==0 ? "" : "same");


    // TKL
    cCentRun->cd(2);
    hTKLCentRun->GetYaxis()->SetRange(i*10+1,(i+1)*10);
    TH1F* hTKLCentRunProj = (TH1F*)hTKLCentRun->ProjectionX();
    hTKLCentRunProj->Divide(hEventsRun);
    hTKLCentRunProj->Scale(hTKLCentRunProj->GetXaxis()->GetNbins()/hTKLCentRunProj->Integral());
    hTKLCentRunProj->SetLineColor(i== 9 ? kViolet : i+1);
    hTKLCentRunProj->SetMinimum(0.5); hTKLCentRunProj->SetMaximum(1.5);
    if(i>4){hTKLCentRunProj->Rebin(2); hTKLCentRunProj->Scale(0.5);}
    hTKLCentRunProj->DrawCopy(i==0 ? "" : "same");
  }
  cCentRun->Update();

  TCanvas *cNevents = new TCanvas(Form("cNevents_%s",trigName.Data()),"N events",1600,1000);
  cNevents->Divide(2,2);
  cNevents->cd(1);
  hNeventsV0M->SetMinimum(0);
  hNeventsV0M->SetLineColor(1);
  hNeventsV0M->DrawCopy();
  cNevents->cd(2);
  hNeventsTKL->SetMinimum(0);
  hNeventsTKL->SetLineColor(1);
  hNeventsTKL->DrawCopy();
  cNevents->cd(3);
  cNevents->cd(4);
  cNevents->Update();

  //*****************************************************************************
  // correlation of multiplicity estimators with Nch in the TPC
  TCanvas *cNch = new TCanvas(Form("cNch_%s",trigName.Data()),"multiplicity estimators vs Nch",1600,1000);
  cNch->Divide(2,2);

  cNch->cd(1);
  hV0MNch->DrawCopy("colz");
  gPad->SetLogz();
  TProfile *tprofx = hV0MNch->ProfileX();
  tprofx->SetLineColor(1);
  tprofx->SetLineWidth(2);
  tprofx->DrawCopy("same");
  cNch->cd(2);
  hTKLNch->DrawCopy("colz");
  gPad->SetLogz();
  tprofx = hTKLNch->ProfileX();
  tprofx->SetLineColor(1);
  tprofx->SetLineWidth(2);
  tprofx->DrawCopy("same");
  cNch->cd(3);
  cNch->cd(4);
  cNch->Update();

  //*********************************************************************************
  // correlation between M/<M> and multiplicity percentile
  TCanvas *cCentPerc = new TCanvas(Form("cCentPerc_%s",trigName.Data()),Form("%s V0M M/<M> versus percentile",trigName.Data()),1600,1000);
  cCentPerc->Divide(2,2);
  cCentPerc->cd(1);
  hV0MCentPerc->DrawCopy("colz");
  TProfile *tpfCentPercX = hV0MCentPerc->ProfileX();
  tpfCentPercX->SetLineColor(1); tpfCentPercX->SetLineWidth(2);
  tpfCentPercX->DrawCopy("same");
  TSpline3 *tspX = new TSpline3(tpfCentPercX);
  tspX->Draw("Lsame");
  Printf("M/<M>         percentile");
  for(Int_t jj = 0; jj < nc+1; jj++)
    Printf("%.2lf         %.3lf",binsc[jj],tspX->Eval(binsc[jj]));
  TProfile *tpfCentPercY = hV0MCentPerc->ProfileY();
  TSpline3 *tspY = new TSpline3(tpfCentPercY);
  Double_t binsperc[12] = {0,0.1,1,5,10,15,20,30,40,50,70,100};
  Printf("percentile         M/<M>");
  for(Int_t jj = 0; jj < 12; jj++)
    Printf("%.1lf        %.3lf",binsperc[jj],tspY->Eval(binsperc[jj]));
  Double_t newbinsc[13] = {0.25,0.5,0.75,1,1.25,1.5,1.75,2,2.5,3,3.5,4,6};
  Printf("new M/<M>         percentile");
  for(Int_t jj = 0; jj < 13; jj++)
    Printf("%.2lf         %.3lf",newbinsc[jj],tspX->Eval(newbinsc[jj]));

  cCentPerc->cd(2);
  hV0MCentPercLog->DrawCopy("colz");
  cCentPerc->cd(3);
  //  hV0MMultMc->DrawCopy("colz");
  gPad->SetLogz();
  cCentPerc->Update();

  TCanvas *cVertexSPD = new TCanvas(Form("cVertexSPD_%s",trigName.Data()),"vertex checks",1600,1000);
  cVertexSPD->Divide(2,2);
  cVertexSPD->cd(1);
  hVertexSPDprimMult->DrawCopy("colz");
  gPad->SetLogz();
  cVertexSPD->cd(2);
  for(Int_t v = 0; v < 7; v++)
  {
    hVertexSPDprimMult->GetYaxis()->SetRange(v*10+1,v*10+10);
    TH1D* hVertexTemp = (TH1D*)hVertexSPDprimMult->ProjectionX();
    hVertexTemp->SetName(Form("hVertexTemp_%i",v));
    hVertexTemp->SetLineColor(v+1);
    if(v==9) hVertexTemp->SetLineColor(kViolet);
    hVertexTemp->DrawCopy(v==0 ? "" : "same");
  }
  gPad->SetLogy();
  cVertexSPD->cd(3);
  hVertexResMult->DrawCopy("colz");
  gPad->SetLogz();
  cVertexSPD->cd(4);
  for(Int_t v = 0; v < 7; v++)
  {
    hVertexResMult->GetYaxis()->SetRange(v*10+1,v*10+10);
    TH1D* hVertexResTemp = (TH1D*)hVertexResMult->ProjectionX();
    hVertexResTemp->SetName(Form("hVertexResTemp_%i",v));
    hVertexResTemp->SetLineColor(v+1);
    if(v==9) hVertexResTemp->SetLineColor(kViolet);
    hVertexResTemp->DrawCopy(v==0 ? "" : "same");
  }
  gPad->SetLogy();

  TCanvas *cVertexQA = new TCanvas(Form("cVertexQA_%s",trigName.Data()),"vertex checks",1600,1000);
  cVertexQA->Divide(2,2);
  cVertexQA->cd(1);
  hVertexContribMult->DrawCopy("colz");
  gPad->SetLogz();
  cVertexQA->cd(2);
  for(Int_t v = 0; v < 7; v++)
  {
    hVertexContribMult->GetYaxis()->SetRange(v*10+1,v*10+10);
    TH1D* hVertexContribTemp = (TH1D*)hVertexContribMult->ProjectionX();
    hVertexContribTemp->SetName(Form("hVertexContribTemp_%i",v));
    hVertexContribTemp->SetLineColor(v+1);
    if(v==9) hVertexContribTemp->SetLineColor(kViolet);
    hVertexContribTemp->DrawCopy(v==0 ? "" : "same");
  }
  gPad->SetLogy();

  TCanvas *cVzNch = new TCanvas(Form("cVzNch_%s",trigName.Data()),"vz versus Nch",800,500);
  cVzNch->cd();
  hVzNch->DrawCopy("colz");
  gPad->SetLogz();
  cVzNch->Update();

  if(1){ // save results
    TString period = filename;
    period.ReplaceAll("eventQA_","");
    period.ReplaceAll(".root","");
    cout << "period = " << period << endl;

    TString pdfname = Form("eventQA_%s_%s.pdf",period.Data(),trigName.Data());

    TIter next(gROOT->GetListOfCanvases());
    TCanvas *ctemp = (TCanvas*)next();
    ctemp->Print(Form("%s(",pdfname.Data()),"pdf");
    ctemp->SaveAs(Form("figures/%s_%s_%s%s",ctemp->GetName(),period.Data(),trigName.Data(),format));
    while ((ctemp = (TCanvas *)next()))
    {
      ctemp->Print(pdfname.Data(),"pdf");
      ctemp->SaveAs(Form("figures/%s_%s_%s%s",ctemp->GetName(),period.Data(),trigName.Data(),format));
    }
    next.Reset();
    ctemp = (TCanvas*)next();
    ctemp->Print(Form("%s]",pdfname.Data()),"pdf");

  }
}
