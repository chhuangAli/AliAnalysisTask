AliAnaPbPbTask *AddPbPbTask(Bool_t usePhysicsSelection=kTRUE, TString outputFileName = "") {

  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    ::Error("AddPbPbTask", "No analysis manager to connect to.");
    return NULL;
  }


  // output file name not defined 
  if (outputFileName.IsNull()) outputFileName = mgr->GetCommonFileName();

 //Create and configure task
  AliAnaPbPbTask *task = new AliAnaPbPbTask("AliAnaPbPbTask");
  if (!task) {
    Error("AddPbPbTask","analysis PbPb task cannot be created! ");
    return NULL;
  }
//  task->GetEventCuts()->SetFilterMask( AliMuonEventCuts::kSelectedTrig | AliMuonEventCuts::kPhysicsSelected | AliMuonEventCuts::kSelectedCentrality);
  task->GetEventCuts()->SetFilterMask( AliMuonEventCuts::kSelectedTrig | AliMuonEventCuts::kPhysicsSelected);
  task->GetEventCuts()->SetTrigClassPatterns("CMUL7-B-NOPF-MUFAST");

  task->GetTrackCuts()->SetFilterMask(AliMuonTrackCuts::kMuEta | AliMuonTrackCuts::kMuThetaAbs | AliMuonTrackCuts::kMuMatchLpt | AliMuonTrackCuts::kMuPdca);
  task->GetTrackCuts()->SetAllowDefaultParams(kTRUE);
  
  // Add task to analysis manager
  mgr->AddTask(task);

  // Connect input container
  mgr->ConnectInput(task, 0, mgr->GetCommonInputContainer());

  // Create and connect output container
  AliAnalysisDataContainer *coutput1 = mgr->CreateContainer( "listOfHisto",   TObjArray::Class(),  AliAnalysisManager::kOutputContainer, outputFileName);
  AliAnalysisDataContainer *coutputEventStat = mgr->CreateContainer("eventCounters", AliCounterCollection::Class(), AliAnalysisManager::kOutputContainer, outputFileName);

  mgr->ConnectOutput(task, 1, coutput1);
  mgr->ConnectOutput(task, 2, coutputEventStat);

  return task;
}
