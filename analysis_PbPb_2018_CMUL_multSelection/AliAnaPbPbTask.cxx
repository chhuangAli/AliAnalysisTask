// ROOT includes
#include "TROOT.h"
#include "TH1.h"
#include "TH2.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TMath.h"
#include "TTree.h"
#include "TNtuple.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TF1.h"
#include "TList.h"
#include "TStyle.h"
#include "TString.h"
#include "TChain.h"

/// ALIROOT/STEER includes
#include "AliInputEventHandler.h"
#include "AliCentrality.h"
#include "AliAODEvent.h"
#include "AliAODTrack.h"

/// ALIROOT/ANALYSIS includes
#include "AliAnalysisManager.h"
#include "AliAnalysisTaskSE.h"
#include "AliCounterCollection.h"

// ALIPHYSICS/PWG includes
#include "AliMuonEventCuts.h"
#include "AliMuonTrackCuts.h"

#include "AliAnaPbPbTask.h"

//Getting centrality
// https://twiki.cern.ch/twiki/bin/view/ALICE/CentralityCodeSnippets
#include "AliMultSelection.h"

#include <iostream>
#include <cmath>
const float c = 3.0;

using std::cout;
using std::endl;

ClassImp(AliAnaPbPbTask);

//__________________________________________________________________________
AliAnaPbPbTask::AliAnaPbPbTask() :
  AliAnalysisTaskSE(),
  fAODEvent(0x0),
  fMuonEventCuts(0x0),
  fMuonTrackCuts(0x0),
  fOutput(0x0),
  fEventCounters(0x0),
  vectorMuon(0),
  treeEventCentrality(0),
  fMultSelection(0x0)
{
  // Default ctor.
}

//__________________________________________________________________________
AliAnaPbPbTask::AliAnaPbPbTask(const char *name) :
  AliAnalysisTaskSE(name),
  fAODEvent(0x0),
  fMuonEventCuts(new AliMuonEventCuts("stdEventCuts","stdEventCuts")),
  fMuonTrackCuts(new AliMuonTrackCuts("stdMuonCuts","stdMuonCuts")),
  fOutput(0x0),
  fEventCounters(0x0),
  vectorMuon(0),
  treeEventCentrality(0),
  fMultSelection(0x0)
{

  // Constructor
  // Define input and output slots here (never in the dummy constructor)
  // Input slot #0 works with a TChain - it is connected to the default input container
  DefineInput(0, TChain::Class());
  // Output slot #1 writes into a TObjArray container
  DefineOutput(1, TObjArray::Class());
  // Output slot #2 writes event counters
  DefineOutput(2, AliCounterCollection::Class());

  //  fMuonTrackCuts->SetFilterMask(AliMuonTrackCuts::kMuEta | AliMuonTrackCuts::kMuThetaAbs | AliMuonTrackCuts::kMuPdca );
  //fMuonTrackCuts->SetFilterMask(AliMuonTrackCuts::kMuEta | AliMuonTrackCuts::kMuThetaAbs | AliMuonTrackCuts::kMuMatchLpt );
  fMuonTrackCuts->SetAllowDefaultParams(kTRUE);

}

//___________________________________________________________________________
/*AliAnalysisTaskSimplePt& AliAnalysisTaskSimplePt::operator=(const AliAnalysisTaskSimplePt& c) 
{
  // Assignment operator
  if (this != &c) {
    AliAnalysisTaskSE::operator=(c) ;
  }
  return *this;
  }*/

//___________________________________________________________________________
/*AliAnalysisTaskSimplePt::AliAnalysisTaskSimplePt(const AliAnalysisTaskSimplePt& c) :
  AliAnalysisTaskSE(c),
  fAODEvent(c.fAODEvent),
  fOutput(c.fOutput)
 {
  // Copy ctor
 }
*/
//___________________________________________________________________________
AliAnaPbPbTask::~AliAnaPbPbTask()
{
  // Destructor. Clean-up the output list, but not the histograms that are put inside
  // (the list is owner and will clean-up these histograms). Protect in PROOF case.
  if (AliAnalysisManager::GetAnalysisManager()->GetAnalysisType() != AliAnalysisManager::kProofAnalysis) {
    if (fOutput) delete fOutput;
    if (fEventCounters) delete fEventCounters;
  }

  delete fMuonTrackCuts;
  delete fMuonEventCuts;

}

//___________________________________________________________________________
void AliAnaPbPbTask::NotifyRun()
{
 /// Notify run
  fMuonTrackCuts->SetRun(fInputHandler);
}


//___________________________________________________________________________
void AliAnaPbPbTask::UserCreateOutputObjects(){

  // Output objects creation
  fOutput = new TObjArray(2000);
  fOutput->SetOwner(); 

  treeEvents = new TTree("eventsTree","tree that contains information of the event");
  treeEvents->Branch("muon",&vectorMuon);
  treeEvents->Branch("eventCentrality",&treeEventCentrality);
  fOutput->AddAtAndExpand(treeEvents,0);

  //TH1I *hEventCentrlty = new TH1I("hEventCentrlty", "Event centrality ; centrality ; Nevents", 10, 0, 100);
  //hEventCentrlty= new TH1F("hEventCentrlty", "Event centrality ; centrality ; Nevents", 100, 0, 100);   
  //hEventCentrlty->Sumw2();
  //fOutput->AddAtAndExpand(hEventCentrlty,1);
  //fOutput->Add(hEventCentrlty);

  // initialize event counters
  fEventCounters = new AliCounterCollection("eventCounters");
  fEventCounters->AddRubric("trigger",1000000);
  fEventCounters->AddRubric("run",1000000);
  fEventCounters->AddRubric("centrality", "m0/0_10/10_20/20_30/30_40/40_50/50_60/60_70/70_80/80_90/90_100/no_centrality");
  fEventCounters->AddRubric("selected","yes/no");
  fEventCounters->Init();
  
  // Required both here and in UserExec()
  PostData(1, fOutput);
  PostData(2, fEventCounters);
} 

//___________________________________________________________________________
void AliAnaPbPbTask::UserExec(Option_t *)
{
  vectorMuon.clear(); // Calling clear is important when submitting jobs on Vaf. It makes sure everytime to clean the memory which is taken up. 
  
  fAODEvent = dynamic_cast<AliAODEvent*> (InputEvent());
  if ( ! fAODEvent ) {
    AliError ("AOD event not found. Nothing done!");
    return;
  }

  //apply event cuts
  Bool_t keepEvent = fMuonEventCuts->IsSelected(fInputHandler);

  //fill event counters
  const TObjArray *trig = fMuonEventCuts->GetSelectedTrigClassesInEvent(InputEvent());
  if (!trig) {
    AliError(Form("ERROR: Could not retrieve list of selected trigger in run %d", fCurrentRunNumber));  
    return;
  }

  double lPercentile = 300; 
  AliMultSelection *fMultSelection = (AliMultSelection * ) fAODEvent->FindListObject("MultSelection");
  if( !fMultSelection) 
  {
      //If you get this warning (and lPercentiles 300) please check that the AliMultSelectionTask actually ran (before your task)
      AliWarning("AliMultSelection object not found!");
  }else
  {
      lPercentile = fMultSelection->GetMultiplicityPercentile("V0M", false);
      //if(lPercentile > 90) 
      cout << "centrality: " << lPercentile <<endl;
      treeEventCentrality = lPercentile;
      //( (TH1F*)fOutput->UncheckedAt(1) )->Fill( lPercentile );
  }

  TString sCentrality;
  sCentrality = classify_centrality_bin_in_alicounter(lPercentile);
  //cout << "sCentrality: " << sCentrality.Data() << endl;


  TString selected = keepEvent ? "yes" : "no";

  TString fillName;

  fillName = Form("trigger:any/run:%d/centrality:%s/selected:%s",
                   fCurrentRunNumber, 
                   sCentrality.Data(), 
                   selected.Data()); 
  fEventCounters->Count(fillName.Data());
  
  for ( Int_t iTrig = 0; iTrig < trig->GetEntries(); iTrig++ )
  {
    TString triggerName = ( (TObjString*) trig->At(iTrig) )->GetString();
    //cout<<"runNr = "<<fCurrentRunNumber<<" "<<iTrig<<" "<<triggerName<<endl;
    fillName = Form("trigger:%s/run:%d/centrality:%s/selected:%s",
                    triggerName.Data(),
                    fCurrentRunNumber,
                    sCentrality.Data(),
                    selected.Data()); 
    fEventCounters->Count(fillName.Data()); 
  }
  
  //keep only selected events by event cuts
  if ( !keepEvent ) return;
  //fMuonEventCuts->Print("mask");
  
  TLorentzVector lvMuon;
  int nTracks = 0;
  nTracks = fAODEvent->GetNumberOfTracks();
  //cout <<"nTracks: "<< nTracks <<"\t";

  int nNUM=0;
  // muon tracks
  for(int iTrack = 0; iTrack < nTracks; iTrack++) 
  {
    AliAODTrack *track1 = (AliAODTrack*) fAODEvent->GetTrack(iTrack);
    if (!track1) 
    {
      AliError(Form("ERROR: Could not retrieve track %d", iTrack));
      continue;
    }
    
//Add cuts in AddPbPbTask.C
//fMuonTrackCuts->SetFilterMask(AliMuonTrackCuts::kMuEta | AliMuonTrackCuts::kMuThetaAbs | AliMuonTrackCuts::kMuMatchLpt);
      if(iTrack == (nTracks-1)) cout<<"\n";
      if ( ! fMuonTrackCuts->IsSelected(track1) ) continue;
//      cout << fMuonTrackCuts->GetSelectionMask(track1) << endl;
//      fMuonTrackCuts->Print("mask"); 
      //( (TH1F*)fOutput->UncheckedAt(1) )->Fill( lPercentile );
      //hEventCentrlty->Fill(lPercentile);
      //cout << "muon charge: " << track1->Charge() << endl;
      lvMuon = MuonTrackToLorentzVector(track1);
      nNUM++;   
      //if(track1->Charge()==0) cout <<":" << track1->Charge()<<"\n";
  
      if(track1->Charge() >0) lvMuon.SetUniqueID(2);
      else if(track1->Charge()<0) lvMuon.SetUniqueID(1);

      //cout <<nNUM << "\t" << iTrack <<"\t"<< lvMuon.Pt() <<"\t"<< track1->Eta()<<"\n";

     vectorMuon.push_back(lvMuon);

      //cout<<" muon1 px= " << lvMuon1.Px() << endl;
  }//end for (int iTrack = 0; iTrack < nTracks; iTrack++) 
    ((TTree*)fOutput->UncheckedAt(0))->Fill();

  // Required both here and in UserCreateOutputObjects()
  PostData(1, fOutput);
  PostData(2, fEventCounters);
}

//___________________________________________________________________________
void AliAnaPbPbTask::Terminate(Option_t *) 
{
  // Display the Pt histogram for dimuon
/*
  fOutput = dynamic_cast<TObjArray*> (GetOutputData(1));
  if (fOutput) {
    TH1F *hDimuonPt = dynamic_cast<TH1F *>( fOutput->FindObject("hDimuonPt") );
    if (hDimuonPt) {
      hDimuonPt->Draw();
    }
  }
*/
  //global statistics
  fEventCounters = static_cast<AliCounterCollection*> (GetOutputData(2));
  if ( fEventCounters ) {
    if (!gROOT->IsBatch() ) {
      //cout<<"Event statistics without any selection "<<endl;
      fEventCounters->Print("trigger/run");
      //cout<<"Event statistics with event selection "<<endl;
      fEventCounters->Print("trigger/run","selected:yes");      
      //new TCanvas();
      //fEventCounters->Draw("run","trigger","selected:yes");
    }
  }

}

TLorentzVector AliAnaPbPbTask::MuonTrackToLorentzVector(const TObject *obj)
{   
    Float_t muonMass = 0.105658369;     
    TLorentzVector lvMuon;                 
 
    const AliVParticle* muonTrack = static_cast<const AliVParticle*> ( obj );      
    Float_t energy = muonMass*muonMass + muonTrack->P()*muonTrack->P();            
    if (energy>0) energy = TMath::Sqrt(energy);                                    
    else energy = -1;                   
    lvMuon.SetPxPyPzE(muonTrack->Px(), muonTrack->Py(), muonTrack->Pz(), energy);  

//    if(charge >0) lvMuon.SetUniqueID(2);
//    else lvMuon.SetUniqueID(1);

    return lvMuon;
}

TString AliAnaPbPbTask::classify_centrality_bin_in_alicounter(double lPercentile)
{

  TString sCentrality;
  if(lPercentile < 0.0 )
  { 
     sCentrality = "m0";
  }else if(lPercentile <= 10.0 && lPercentile >=0.0)
  {
     sCentrality = "0_10";
     //cout << "sCentrality: " << sCentrality.Data() << endl;
   }else if(lPercentile <= 20.0 && lPercentile >10.0)
   {
     sCentrality = "10_20";
     //cout << "sCentrality: " << sCentrality.Data() << endl;
   }else if(lPercentile <= 30.0 && lPercentile >20.0)
   {
     sCentrality = "20_30";
     //cout << "sCentrality: " << sCentrality.Data() << endl;
   }else if(lPercentile <= 40.0 && lPercentile >30.0)
   {
     sCentrality = "30_40";
     //cout << "sCentrality: " << sCentrality.Data() << endl;
   }else if(lPercentile <= 50.0 && lPercentile >40.0)
   {
       sCentrality = "40_50";
       //cout << "sCentrality: " << sCentrality.Data() << endl;
    }else if(lPercentile <= 60.0 && lPercentile >50.0)
    {
       sCentrality = "50_60";
       //cout << "sCentrality: " << sCentrality.Data() << endl;
    }else if(lPercentile <= 70.0 && lPercentile >60.0)
    {
       sCentrality = "60_70";
       //cout << "sCentrality: " << sCentrality.Data() << endl;
    }else if(lPercentile <= 80.0 && lPercentile >70.0)
    {
       sCentrality = "70_80";
       //cout << "sCentrality: " << sCentrality.Data() << endl;
    }else if(lPercentile <= 90.0 && lPercentile >80.0)
    {
       sCentrality = "80_90";
       //cout << "sCentrality: " << sCentrality.Data() << endl;
    }else if(lPercentile <= 110.0 && lPercentile >90.0)
    {
       sCentrality = "90_100";
       //cout << "sCentrality: " << sCentrality.Data() << endl;
    }
    else{ sCentrality="no_centrality"; }
    

 return sCentrality;
}
