#ifndef AliAnaPbPbTask_H
#define AliAnaPbPbTask_H
#include "AliAnalysisTaskSE.h"

class TH1F;
class TTree;
class TNtuple;
class TList;
class TObjArray;
class AliMuonEventCuts;
class AliMuonTrackCuts;
class AliCounterCollection;
class AliMultSelection;
class AliVParticle;
class AliAODEvent;
class TLorentzVector;

class AliAnaPbPbTask : public AliAnalysisTaskSE {

  public:

    AliAnaPbPbTask();
    AliAnaPbPbTask(const char *name);
    virtual ~AliAnaPbPbTask();

    virtual void NotifyRun();
    virtual void UserCreateOutputObjects();
    virtual void UserExec(Option_t *option);
    virtual void Terminate(Option_t *);
    
    /// Get track cuts
    AliMuonTrackCuts* GetTrackCuts() { return fMuonTrackCuts; }
    /// Get event cuts
    AliMuonEventCuts* GetEventCuts() { return fMuonEventCuts;}

    TLorentzVector MuonTrackToLorentzVector(const TObject *obj);
    TString classify_centrality_bin_in_alicounter(double );
    

    
 private:
    
    //AliAnalysisTaskSimplePt(const AliAnalysisTaskSimplePt&);
    // AliAnalysisTaskSimplePt& operator=(const AliAnalysisTaskSimplePt&);

//    enum eList { kDimuonPt=1, kDimuonInvM=2, kDimuonY=3, kDimuonYCut=4, kEventCentrlty=5};

    AliAODEvent* fAODEvent;       // AOD event

    AliMuonEventCuts *fMuonEventCuts; //< Event cuts
    AliMuonTrackCuts *fMuonTrackCuts; //< Track cuts

    TTree *treeEvents; //!
    //TH1F *hEventCentrlty;
    std::vector<TLorentzVector> vectorMuon;
    double treeEventCentrality;

    TObjArray *fOutput;               //!< List of histograms for data
    AliMultSelection *fMultSelection;    
    AliCounterCollection *fEventCounters; //!< Event statistics

    ClassDef(AliAnaPbPbTask, 1);

};

#endif
