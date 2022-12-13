//R__ADD_INCLUDE_PATH("/disk/PublicMCTrees/CustomClasses_v2/");
R__ADD_INCLUDE_PATH("/Users/vytautas/Stuff/Correlations/JewelAnalysis/CustomClasses_v2/");
#include "Include.h"
#include "TTree.h"
#include "TFile.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TClonesArray.h"
#include "TString.h"
#include "TMath.h"
#include "TAxis.h"
#include "TROOT.h"
#include "Scripts/SinglePool.h"
#define C_PI_HALF 1.5707963
#define C_PI_TH 4.7123890
#define C_TWOPI 6.2831853
TTree *tr=0;
MyEvent *mev;
TClonesArray *tca;
MyParticle *mpa;
Double_t gMinPt=0.5;
Double_t gMaxPt=6;

Double_t *ptBins, *centBins, *phiBins, *etaBins;
Int_t NptBins, NcentBins, NphiBins, NetaBins;

TAxis *centAxis;
SinglePool *sp;
Double_t lPhi, lEta;
TH3D *h3, *hMix;
TH2D *hTrigPart;
TH1D *h1, *hdEta;
Int_t nMixEv=50;
void RQT() { gROOT->ProcessLine(".q"); };
Double_t *makeBins(vector<Double_t> inVals, Int_t &nBins) {
  nBins = inVals.size()-1;
  Double_t *retb = new Double_t[nBins+1];
  for(Int_t i=0;i<=nBins;i++) retb[i]=inVals[i];
  return retb;
};
Double_t *makeBins(Double_t bMin, Double_t bMax, Int_t nBins) {
  Double_t *retb = new Double_t[nBins+1];
  for(Int_t i=0;i<=nBins;i++) retb[i] = bMin+(bMax-bMin)/nBins*i;
  return retb;
};
void fixPhi(Double_t &inPhi) { if(inPhi<-C_PI_HALF) inPhi+=C_TWOPI; else if(inPhi>C_PI_TH) inPhi-=C_TWOPI; };
TH2* MakeDebugPlot();
Bool_t Init(TString infile) {
  TFile *tf = new TFile(infile.Data(),"READ");
  if(!tf) return kFALSE;
  tr = (TTree*)tf->Get("TT");
  if(!tf) return kFALSE;
  tr->SetBranchAddress("event",&mev);
  tr->SetBranchAddress("tracks",&tca);
  ptBins = makeBins({0.5,1.,2.,3.,4.,6.},NptBins);
  NphiBins=20;
  phiBins = makeBins(-C_PI_HALF,C_PI_TH,NphiBins);
  NetaBins=32;
  etaBins = makeBins(-1.6,1.6,NetaBins);
  h3 = new TH3D("CorrMatrix","Corr; pt; dphi; deta",NptBins,ptBins,NphiBins,phiBins,NetaBins,etaBins);
  hMix = new TH3D("MixEvent","Mix; pt; dphi; deta",NptBins,ptBins,NphiBins,phiBins,NetaBins,etaBins);
  hTrigPart = new TH2D("TriggerPart","Trigger Particle; dphi; deta",NphiBins,phiBins,NetaBins,etaBins);
  hdEta = new TH1D("dNdeta","dNdeta",NetaBins,etaBins);
  h1 = new TH1D("TrigCount","TrigCount; cent; Nev",1,0,100);
  sp = new SinglePool(nMixEv); //keep nMixEv tracks from prev. events
  return kTRUE;
};
Int_t findLeadingTrack(Double_t ptMin, Double_t ptMax, Double_t &lPhi, Double_t &lEta) {
  Int_t retInd=-1;
  Double_t maxPt=0;
  if(!tca) { printf("TCA not available!\n"); return retInd; };
  for(Int_t i=0;i<tca->GetEntries(); i++) {
    mpa = (MyParticle*)tca->At(i);
    if(TMath::Abs(mpa->fEta)>0.8) continue;
    Double_t lpt = mpa->fPt;
    if(lpt<ptMin || lpt>ptMax) continue;
    if(lpt<maxPt) continue;
    maxPt=lpt;
    retInd=i;
    lPhi=mpa->fPhi;
    lEta=mpa->fEta;
  };
  return retInd;
};
void MakeCorrHist(TString inFile) {
  if(!Init(inFile)) RQT();
  Long64_t tent=tr->GetEntries();
  for(Long64_t i=0; i<tent; i++) {
    tr->GetEntry(i);
    Int_t lInd = findLeadingTrack(8.,15.,lPhi,lEta);
    if(lInd<0) {lPhi=0; lEta=0;};//continue;
    hTrigPart->Fill(lPhi>C_PI_TH?(lPhi-C_TWOPI):lPhi,lEta);
    Int_t V0MAmp = 10;
    h1->Fill(V0MAmp);
    for(Int_t i=0;i<tca->GetEntries();i++) {
      mpa = (MyParticle*)tca->At(i);
      if(TMath::Abs(mpa->fEta)>0.8) continue;
      if(mpa->fPt<gMinPt || mpa->fPt>gMaxPt) continue;
      Double_t tr_Phi = mpa->fPhi;
      Double_t tr_Eta = mpa->fEta;
      Double_t tr_Pt  = mpa->fPt;
      hdEta->Fill(tr_Eta);
      Double_t dPhi=tr_Phi-lPhi;
      fixPhi(dPhi);
      h3->Fill(tr_Pt,dPhi,tr_Eta-lEta);
      //Fill mixed event
      if(!sp->IsReady()) continue;
      for(SPEnt j: sp->fVec) {
	       dPhi = tr_Phi-j.first;
	       fixPhi(dPhi);
	       hMix->Fill(tr_Pt,dPhi,tr_Eta-j.second);
      };
    };
    //Push to event pool -- pushing leading track isntead of all the shit, and calculate for every event
    //also, I might want to disable this for now, I think this shouldn't affect the measurement since it doesn't have any detector effects anyways. But the problem is in eta structure :/
    sp->AddValue(make_pair(lPhi,lEta));
  };
  TString outFN=inFile;
  int indx=outFN.Index("JEWEL_");
  int sizx=outFN.Length();
  outFN=outFN(indx+1,sizx-indx-1);
  indx=outFN.Index("_");
  sizx=outFN.Length();
  outFN=outFN(indx+1,sizx-indx-1);
  outFN.Prepend("Processed/File_");
  outFN = "testOut.root";
  TFile *fOut = new TFile(outFN.Data(),"RECREATE");
  h3->Write();
  hMix->Write();
  h1->Write();
  hTrigPart->Write();
  TH2 *hDebug = MakeDebugPlot();
  hDebug->Write();
  hdEta->Write();
  fOut->Close();
  RQT();
};
TH2 *MakeDebugPlot() {
  h3->GetXaxis()->SetRange(1,1);
  TH2 *smev = (TH2*)h3->Project3D("zy");
  smev = (TH2*)smev->Clone("Ratio");
  smev->SetDirectory(0);
  h3->GetXaxis()->SetRange(1,h3->GetNbinsX());
  hMix->GetXaxis()->SetRange(1,1);
  TH2 *mxev = (TH2*)hMix->Project3D("zy");
  mxev = (TH2*)mxev

  ->Clone("MixedProj");
  mxev->SetDirectory(0);
  hMix->GetXaxis()->SetRange(1,hMix->GetNbinsX());
  smev->Divide(mxev);
  delete mxev;
  return smev;
}
