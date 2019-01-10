#if !defined (__CINT__) || defined (__CLING__)
#include "AliAnalysisAlien.h"
#include "AliAnalysisManager.h"
#include "AliAODInputHandler.h"
#include "AliAnaPbPbTask.h"

R__ADD_INCLUDE_PATH($ALICE_ROOT)
R__ADD_INCLUDE_PATH($ALICE_PHYSICS)
#include <OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C>

#endif

void runAnalysis(const char *runListFileName = "LHC18r_runList07.txt", Bool_t local = kFALSE, Bool_t gridTest = kFALSE, int N_event =1000)
{
    // set if you want to run the analysis locally (kTRUE), or on grid (kFALSE)
 //   Bool_t local = kTRUE;
    // if you run on grid, specify test mode (kTRUE) or full grid model (kFALSE)
//    Bool_t gridTest = kFALSE;

//    int N_event =1000;

    Bool_t isPhySel=kTRUE;
    
    // since we will compile a class, tell root where to look for headers  
#if !defined (__CINT__) || defined (__CLING__)
    gInterpreter->ProcessLine(".include $ROOTSYS/include");
    gInterpreter->ProcessLine(".include $ALICE_ROOT/include");
//    gInterpreter->ProcessLine(".include $ALICE_PHYSICS/OADB/COMMON/MULTIPLICITY");
//    gInterpreter->ProcessLine(".include $ALICE_PHYSICS/include");    
#else
    gROOT->ProcessLine(".include $ROOTSYS/include");
    gROOT->ProcessLine(".include $ALICE_ROOT/include");
//    gROOT->ProcessLine(".include $ALICE_PHYSICS/OADB/COMMON/MULTIPLICITY");
    gROOT->ProcessLine(".include $ALICE_PHYSICS/include");
#endif

     
    gSystem->Load("libSTEERBase");
    gSystem->Load("libESD");
    gSystem->Load("libAOD");
    gSystem->Load("libANALYSIS");
    gSystem->Load("libANALYSISalice");
    gSystem->Load("libANALYSISaliceBase");
    gSystem->Load("libCORRFW");
    gSystem->Load("libOADB");
    // create the analysis manager
    AliAnalysisManager *mgr = new AliAnalysisManager("CMULEventAnalysis");
    AliAODInputHandler *aodH = new AliAODInputHandler();
    mgr->SetInputEventHandler(aodH);

    // compile the class and load the add task macro
    // here we have to differentiate between using the just-in-time compiler
    // from root6, or the interpreter of root5
#if !defined (__CINT__) || defined (__CLING__)
    //Here is ROOT6 
    //gInterpreter->LoadMacro(Form("%s/OADB/COMMON/MULTIPLICITY/AliMultSelectionTask.cxx++g",gSystem->ExpandPathName("$ALICE_PHYSICS")));
    //gInterpreter->LoadMacro(Form("%s/OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C",gSystem->ExpandPathName("$ALICE_PHYSICS")));
    //AliMultSelectionTask * MultSelTask = reinterpret_cast<AliMultSelectionTask*>(gInterpreter->ExecuteMacro(Form("%s/OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C",gSystem->ExpandPathName("$ALICE_PHYSICS")) ));
    AliMultSelectionTask *MultSelTask = AddTaskMultSelection(kFALSE);
    //MultSelTask-> SetAlternateOADBforEstimators(Form("LHC18q") ); //Look for the corresponding OADB file
    //MultSelTask->SetUseDefaultCalib(kTRUE); //Not needed if the calibration is done. 

    gInterpreter->LoadMacro("AliAnaPbPbTask.cxx++g");
    AliAnaPbPbTask *CMULEventTask = reinterpret_cast<AliAnaPbPbTask*>(gInterpreter->ExecuteMacro("AddPbPbTask.C"));
#else
    // Here is ROOT5
    //gROOT->LoadMacro(Form("%s/OADB/COMMON/MULTIPLICITY/AliMultSelectionTask.cxx++g",gSystem->ExpandPathName("$ALICE_PHYSICS")));
    gROOT->LoadMacro(Form("%s/OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C",gSystem->ExpandPathName("$ALICE_PHYSICS")));
    AliMultSelectionTask *MultSelTask = AddTaskMultSelection(kFALSE);
    //MultSelTask-> SetAlternateOADBforEstimators(Form("LHC18q") );    
    //MultSelTask->SetUseDefaultCalib(kTRUE);

    gROOT->LoadMacro("AliAnaPbPbTask.cxx++g");
    gROOT->LoadMacro("AddPbPbTask.C");
    AliAnaPbPbTask *CMULEventTask = AddPbPbTask(isPhySel, "AnalysisResults.root");
#endif


    if(!mgr->InitAnalysis()) return;
    mgr->SetDebugLevel(2);
    mgr->PrintStatus();
    mgr->SetUseProgressBar(1, 25);

    if(local) {
        // if you want to run locally, we need to define some input
        TChain* chain = new TChain("aodTree");
        // add a few files to the chain (change this so that your local files are added)
//        chain->Add("$HOME/local/AliAOD_PbPbRawData/2018/LHC18q/000295584/muon_calo_pass1/AOD/002/AliAOD.root");
        chain->Add("$HOME/public/RawData_2018/2018/LHC18q/000295584/muon_calo_pass1/AOD/002/AliAOD.root");
//        chain->Add("$HOME/local/AliAOD_PbPbRawData/2018/000295584/muon_calo_pass1/AOD/001/AliAOD.Muons.root");
//        chain->Add("$HOME/local/AliAOD_PbPbRawData/2015/muon_calo_pass1/AOD175/3825/AliAOD.Muons.root");
        // start the analysis locally, reading the events from the tchain
        mgr->StartAnalysis("local", chain);
    } else {
        // if we want to run on grid, we create and configure the plugin
        AliAnalysisAlien *alienHandler = new AliAnalysisAlien();
        // also specify the include (header) paths on grid
        alienHandler->AddIncludePath("-I. -I$ROOTSYS/include -I$ALICE_ROOT -I$ALICE_ROOT/include -I$ALICE_PHYSICS/include -I$ALICE_PHYSICS/OADB/COMMON/MULTIPLICITY");
        // make sure your source files get copied to grid
        alienHandler->SetAdditionalLibs("AliAnaPbPbTask.cxx AliAnaPbPbTask.h");
        alienHandler->SetAnalysisSource("AliAnaPbPbTask.cxx");
        // select the aliphysics version. all other packages
        // are LOADED AUTOMATICALLY!
//        alienHandler->SetAliPhysicsVersion("vAN-20180906-1");
        alienHandler->SetAliPhysicsVersion("vAN-20181209-1");
//        alienHandler->SetAliPhysicsVersion("vAN-20181211_ROOT6-1");
        // set the Alien API version
        alienHandler->SetAPIVersion("V1.1x");
        // select the input data
        alienHandler->SetGridDataDir("/alice/data/2018/LHC18r");
        //alienHandler->SetDataPattern("muon_calo_pass1/AOD/*/AliAOD.Muons.root");
        alienHandler->SetDataPattern("muon_calo_pass1/AOD/*/AliAOD.root");
        // MC has no prefix, data has prefix 000
        alienHandler->SetRunPrefix("000");

        int runNumber=0;
        std::ifstream input_file; input_file.open(runListFileName);
        while(true)
        {
          input_file >> runNumber;
          if(input_file.eof()) break;

          std::cout << "===========================" << std::endl;
          std::cout << "Run " << runNumber << " added" << std::endl;
          
          alienHandler->AddRunNumber(runNumber);
          runNumber=0;
        }
        input_file.close();
        
        //alienHandler->AddRunNumber(295584);
        // number of files per subjob
        alienHandler->SetSplitMaxInputFileNumber(10);
        alienHandler->SetExecutable("myTask.sh");
        // specify how many seconds your job may take
        alienHandler->SetTTL(20000); // unit: second 
        alienHandler->SetJDLName("myTask.jdl");

        alienHandler->SetOutputToRunNo(kTRUE);
        alienHandler->SetKeepLogs(kTRUE);
        // merging: run with kTRUE to merge on grid
        // after re-running the jobs in SetRunMode("terminate") 
        // (see below) mode, set SetMergeViaJDL(kFALSE) 
        // to collect final results
        alienHandler->SetMaxMergeStages(1);
        alienHandler->SetMergeViaJDL(kTRUE);
//        alienHandler->SetMergeViaJDL(kFALSE);

        // define the output folders
        alienHandler->SetGridWorkingDir("LHC18r");
        alienHandler->SetGridOutputDir("myOutputDir");

        // connect the alien plugin to the manager
        mgr->SetGridHandler(alienHandler);
        if(gridTest) {
            // speficy on how many files you want to run
            alienHandler->SetNtestFiles(1);
            // and launch the analysis
            alienHandler->SetRunMode("test");
            mgr->StartAnalysis("grid",N_event);
        } else {
            // else launch the full grid analysis
            alienHandler->SetRunMode("full");
        //    alienHandler->SetRunMode("terminate");
            mgr->StartAnalysis("grid");
        }
    }
}
