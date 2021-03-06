#include <iostream>
#include <fstream>

#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"
#include "TDirectory.h"

using namespace std;

//Global stuff
bool verbose = true;
int current_open_file = -1;
TFile * aod_file;
const int max_events = 10;
Long64_t aod_event_start = 0;
Long64_t aod_event_start_offset = -200;

Long64_t nmatched = 0;
Long64_t nunmatched = 0;
Long64_t ntotal = 0;

//--new branches for merged tree
Bool_t         matched;
Int_t          AODRun_;
Long64_t       AODEvent_;
vector<int>    AODllpId_;
vector<int>    AODllpStatus_;
vector<float>  AODllpPt_;
vector<float>  AODllpEta_;
vector<float>  AODllpPhi_;
vector<float>  AODllpMass_;
vector<int>    AODllpDaughterId_;
vector<int>    AODllpDaughterStatus_;
vector<float>  AODllpDaughterPt_;
vector<float>  AODllpDaughterEta_;
vector<float>  AODllpDaughterPhi_;
vector<float>  AODllpDaughterMass_;
Int_t          AODnCaloJet_;
vector<float>  AODCaloJetPt_;
vector<float>  AODCaloJetEta_;
vector<float>  AODCaloJetPhi_;
vector<float>  AODCaloJetAlphaMax_;
vector<float>  AODCaloJetAlphaMax2_;
vector<float>  AODCaloJetAlphaMaxPrime_;
vector<float>  AODCaloJetAlphaMaxPrime2_;
vector<float>  AODCaloJetBeta_;
vector<float>  AODCaloJetBeta2_;
vector<float>  AODCaloJetSumIP_;
vector<float>  AODCaloJetSumIPSig_;
vector<float>  AODCaloJetMedianIP_;
vector<float>  AODCaloJetMedianLog10IPSig_;
vector<float>  AODCaloJetTrackAngle_;
vector<float>  AODCaloJetLogTrackAngle_;
vector<float>  AODCaloJetMedianLog10TrackAngle_;
vector<float>  AODCaloJetTotalTrackAngle_;
vector<float>  AODCaloJetAvfVx_;
vector<float>  AODCaloJetAvfVy_;
vector<float>  AODCaloJetAvfVz_;
vector<float>  AODCaloJetAvfVertexTotalChiSquared_;
vector<float>  AODCaloJetAvfVertexDegreesOfFreedom_;
vector<float>  AODCaloJetAvfVertexChi2NDoF_;
vector<float>  AODCaloJetAvfVertexDistanceToBeam_;
vector<float>  AODCaloJetAvfVertexTransverseError_;
vector<float>  AODCaloJetAvfVertexTransverseSig_;
vector<float>  AODCaloJetAvfVertexDeltaEta_;
vector<float>  AODCaloJetAvfVertexDeltaPhi_;
vector<float>  AODCaloJetAvfVertexRecoilPt_;
vector<float>  AODCaloJetAvfVertexTrackMass_;
vector<float>  AODCaloJetAvfVertexTrackEnergy_;
vector<float>  AODCaloJetAvfBeamSpotDeltaPhi_;
vector<float>  AODCaloJetAvfBeamSpotRecoilPt_;
vector<float>  AODCaloJetAvfBeamSpotMedianDeltaPhi_;
vector<float>  AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_;
vector<int>    AODCaloJetNCleanMatchedTracks_;
vector<int>    AODCaloJetSumHitsInFrontOfVert_;
vector<int>    AODCaloJetSumMissHitsAfterVert_;
vector<int>    AODCaloJetHitsInFrontOfVertPerTrack_;
vector<int>    AODCaloJetMissHitsAfterVertPerTrack_;
vector<float>  AODCaloJetAvfDistToPV_;
vector<float>  AODCaloJetAvfVertexDeltaZtoPV_;
vector<float>  AODCaloJetAvfVertexDeltaZtoPV2_;


int find_aod(std::vector<TString> aod_list, unsigned int &start_suggest, int mini_run, Long64_t mini_event){

  //Loop over AOD file list
  unsigned int N = aod_list.size();
  for(unsigned int i=0; i<N; i++){

    //Start with suggestion from last match
    unsigned int f = start_suggest + i;
    if(f >= N ) f = f - N; 
    if(verbose && i==0) cout << "Starting search for Run " << mini_run << " Event " << mini_event << " at file " << f << endl;
    if(verbose) cout << "Loop: " << f << endl;

    if(current_open_file<0 || (unsigned int)current_open_file!=f){
      if(current_open_file>=0) aod_file->Close();
      current_open_file = f;
      aod_file = TFile::Open("root://cmseos.fnal.gov://"+aod_list[f], "READ");
      if(aod_file->IsZombie()){
	cout << "aod_file is zombie" << endl;
	return 0;
      }
    }
    
    //Only get what you need from AOD file
    TTree *aod_tree = (TTree*)aod_file->Get("lldjNtuple/EventTree");
    if(aod_tree->IsZombie()){
      cout << "aod_tree is zombie" << endl;
      return 0;
    }
    aod_tree->SetBranchStatus("*",     0);
    aod_tree->SetBranchStatus("run",   1);
    aod_tree->SetBranchStatus("event", 1);
    
    Long64_t aod_event;
    int aod_run;
    aod_tree->SetBranchAddress("run", &aod_run);
    aod_tree->SetBranchAddress("event", &aod_event);
    
    //Loop through AOD tree
    Long64_t nentries = aod_tree->GetEntries();
    for(Long64_t j = 0; j < nentries; j++) {
      
      //Start search X events offset from previous match (recommend negative values)
      Long64_t event_to_get = aod_event_start + j + aod_event_start_offset;
      if(event_to_get<0) event_to_get = nentries + event_to_get;//event_to_get is negative
      if(event_to_get >= nentries) event_to_get = event_to_get - nentries;
      
      aod_tree->GetEntry(event_to_get);
      
      if(aod_run != mini_run) continue;
      if(aod_event != mini_event) continue;

      //Matched!
      if(verbose) cout << "Found match: File = " << aod_list[f] << ", " << aod_run << " " << mini_run << " " << aod_event << " " << mini_event << endl;
      //cout << "start: " << aod_event_start << ", and found: " << event_to_get << endl;
      //if(event_to_get<(aod_event_start+aod_event_start_offset)) cout << "HAD TO LOOP!" << endl;

      start_suggest = f;
      aod_event_start = event_to_get;

      //Now get everything
      aod_tree->SetBranchStatus("llpId", 1); 
      aod_tree->SetBranchStatus("llpStatus", 1); 
      aod_tree->SetBranchStatus("llpPt", 1);     
      aod_tree->SetBranchStatus("llpEta", 1);    
      aod_tree->SetBranchStatus("llpPhi", 1);    
      aod_tree->SetBranchStatus("llpMass", 1);   
      aod_tree->SetBranchStatus("llpDaughterId", 1);     
      aod_tree->SetBranchStatus("llpDaughterStatus", 1);
      aod_tree->SetBranchStatus("llpDaughterPt", 1);    
      aod_tree->SetBranchStatus("llpDaughterEta", 1);   
      aod_tree->SetBranchStatus("llpDaughterPhi", 1);   
      aod_tree->SetBranchStatus("llpDaughterMass", 1);  
      aod_tree->SetBranchStatus("AODnCaloJet", 1);
      aod_tree->SetBranchStatus("AODCaloJetPt", 1);
      aod_tree->SetBranchStatus("AODCaloJetEta", 1);
      aod_tree->SetBranchStatus("AODCaloJetPhi", 1);
      aod_tree->SetBranchStatus("AODCaloJetAlphaMax", 1);
      aod_tree->SetBranchStatus("AODCaloJetAlphaMax2", 1);
      aod_tree->SetBranchStatus("AODCaloJetAlphaMaxPrime", 1);
      aod_tree->SetBranchStatus("AODCaloJetAlphaMaxPrime2", 1);
      aod_tree->SetBranchStatus("AODCaloJetBeta", 1);          
      aod_tree->SetBranchStatus("AODCaloJetBeta2", 1);         
      aod_tree->SetBranchStatus("AODCaloJetSumIP", 1);         
      aod_tree->SetBranchStatus("AODCaloJetSumIPSig", 1);      
      aod_tree->SetBranchStatus("AODCaloJetMedianIP", 1);      
      aod_tree->SetBranchStatus("AODCaloJetMedianLog10IPSig", 1);
      aod_tree->SetBranchStatus("AODCaloJetTrackAngle", 1);     
      aod_tree->SetBranchStatus("AODCaloJetLogTrackAngle", 1);  
      aod_tree->SetBranchStatus("AODCaloJetMedianLog10TrackAngle", 1);
      aod_tree->SetBranchStatus("AODCaloJetTotalTrackAngle", 1);   
      aod_tree->SetBranchStatus("AODCaloJetAvfVx", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVy", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVz", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexTotalChiSquared", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexDegreesOfFreedom", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexChi2NDoF", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexDistanceToBeam", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexTransverseError", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexTransverseSig", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexDeltaEta", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexDeltaPhi", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexRecoilPt", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexTrackMass", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexTrackEnergy", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfBeamSpotDeltaPhi", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfBeamSpotRecoilPt", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfBeamSpotMedianDeltaPhi", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfBeamSpotLog10MedianDeltaPhi", 1);
      aod_tree->SetBranchStatus("AODCaloJetNCleanMatchedTracks", 1);
      aod_tree->SetBranchStatus("AODCaloJetSumHitsInFrontOfVert", 1);
      aod_tree->SetBranchStatus("AODCaloJetSumMissHitsAfterVert", 1);
      aod_tree->SetBranchStatus("AODCaloJetHitsInFrontOfVertPerTrack", 1);
      aod_tree->SetBranchStatus("AODCaloJetMissHitsAfterVertPerTrack", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfDistToPV", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexDeltaZtoPV", 1);
      aod_tree->SetBranchStatus("AODCaloJetAvfVertexDeltaZtoPV2", 1);

      vector<int>    *orig_AODllpId_ = new std::vector<int>();
      vector<int>    *orig_AODllpStatus_ = new std::vector<int>();
      vector<float>  *orig_AODllpPt_ = new std::vector<float>();
      vector<float>  *orig_AODllpEta_ = new std::vector<float>();
      vector<float>  *orig_AODllpPhi_ = new std::vector<float>();
      vector<float>  *orig_AODllpMass_ = new std::vector<float>();
      vector<int>    *orig_AODllpDaughterId_ = new std::vector<int>();
      vector<int>    *orig_AODllpDaughterStatus_ = new std::vector<int>();
      vector<float>  *orig_AODllpDaughterPt_ = new std::vector<float>();
      vector<float>  *orig_AODllpDaughterEta_ = new std::vector<float>();
      vector<float>  *orig_AODllpDaughterPhi_ = new std::vector<float>();
      vector<float>  *orig_AODllpDaughterMass_ = new std::vector<float>();
      Int_t           orig_AODnCaloJet_ = -1;
      vector<float>  *orig_AODCaloJetPt_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetEta_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetPhi_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAlphaMax_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAlphaMax2_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAlphaMaxPrime_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAlphaMaxPrime2_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetBeta_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetBeta2_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetSumIP_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetSumIPSig_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetMedianIP_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetMedianLog10IPSig_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetTrackAngle_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetLogTrackAngle_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetMedianLog10TrackAngle_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetTotalTrackAngle_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVx_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVy_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVz_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexTotalChiSquared_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexDegreesOfFreedom_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexChi2NDoF_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexDistanceToBeam_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexTransverseError_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexTransverseSig_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexDeltaEta_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexDeltaPhi_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexRecoilPt_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexTrackMass_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexTrackEnergy_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfBeamSpotDeltaPhi_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfBeamSpotRecoilPt_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfBeamSpotMedianDeltaPhi_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_ = new std::vector<float>();
      vector<int>    *orig_AODCaloJetNCleanMatchedTracks_ = new std::vector<int>();
      vector<int>    *orig_AODCaloJetSumHitsInFrontOfVert_ = new std::vector<int>();
      vector<int>    *orig_AODCaloJetSumMissHitsAfterVert_ = new std::vector<int>();
      vector<int>    *orig_AODCaloJetHitsInFrontOfVertPerTrack_ = new std::vector<int>();
      vector<int>    *orig_AODCaloJetMissHitsAfterVertPerTrack_ = new std::vector<int>();
      vector<float>  *orig_AODCaloJetAvfDistToPV_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexDeltaZtoPV_ = new std::vector<float>();
      vector<float>  *orig_AODCaloJetAvfVertexDeltaZtoPV2_ = new std::vector<float>();
      aod_tree->SetBranchAddress("llpId",              &orig_AODllpId_);
      aod_tree->SetBranchAddress("llpStatus",          &orig_AODllpStatus_);
      aod_tree->SetBranchAddress("llpPt",              &orig_AODllpPt_);
      aod_tree->SetBranchAddress("llpEta",             &orig_AODllpEta_);
      aod_tree->SetBranchAddress("llpPhi",             &orig_AODllpPhi_);
      aod_tree->SetBranchAddress("llpMass",            &orig_AODllpMass_);
      aod_tree->SetBranchAddress("llpDaughterId",      &orig_AODllpDaughterId_);
      aod_tree->SetBranchAddress("llpDaughterStatus",  &orig_AODllpDaughterStatus_);
      aod_tree->SetBranchAddress("llpDaughterPt",      &orig_AODllpDaughterPt_);
      aod_tree->SetBranchAddress("llpDaughterEta",     &orig_AODllpDaughterEta_);
      aod_tree->SetBranchAddress("llpDaughterPhi",     &orig_AODllpDaughterPhi_);
      aod_tree->SetBranchAddress("llpDaughterMass",    &orig_AODllpDaughterMass_);
      aod_tree->SetBranchAddress("AODnCaloJet",                                 &orig_AODnCaloJet_);
      aod_tree->SetBranchAddress("AODCaloJetPt",                                &orig_AODCaloJetPt_);
      aod_tree->SetBranchAddress("AODCaloJetEta",                               &orig_AODCaloJetEta_);
      aod_tree->SetBranchAddress("AODCaloJetPhi",                               &orig_AODCaloJetPhi_);
      aod_tree->SetBranchAddress("AODCaloJetAlphaMax",                          &orig_AODCaloJetAlphaMax_);
      aod_tree->SetBranchAddress("AODCaloJetAlphaMax2",                         &orig_AODCaloJetAlphaMax2_);
      aod_tree->SetBranchAddress("AODCaloJetAlphaMaxPrime",                     &orig_AODCaloJetAlphaMaxPrime_);
      aod_tree->SetBranchAddress("AODCaloJetAlphaMaxPrime2",                    &orig_AODCaloJetAlphaMaxPrime2_);
      aod_tree->SetBranchAddress("AODCaloJetBeta",                              &orig_AODCaloJetBeta_);
      aod_tree->SetBranchAddress("AODCaloJetBeta2",                             &orig_AODCaloJetBeta2_);
      aod_tree->SetBranchAddress("AODCaloJetSumIP",                             &orig_AODCaloJetSumIP_);
      aod_tree->SetBranchAddress("AODCaloJetSumIPSig",                          &orig_AODCaloJetSumIPSig_);
      aod_tree->SetBranchAddress("AODCaloJetMedianIP",                          &orig_AODCaloJetMedianIP_);
      aod_tree->SetBranchAddress("AODCaloJetMedianLog10IPSig",                  &orig_AODCaloJetMedianLog10IPSig_);
      aod_tree->SetBranchAddress("AODCaloJetTrackAngle",                        &orig_AODCaloJetTrackAngle_);
      aod_tree->SetBranchAddress("AODCaloJetLogTrackAngle",                     &orig_AODCaloJetLogTrackAngle_);
      aod_tree->SetBranchAddress("AODCaloJetMedianLog10TrackAngle",             &orig_AODCaloJetMedianLog10TrackAngle_);
      aod_tree->SetBranchAddress("AODCaloJetTotalTrackAngle",                   &orig_AODCaloJetTotalTrackAngle_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVx",                             &orig_AODCaloJetAvfVx_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVy",                             &orig_AODCaloJetAvfVy_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVz",                             &orig_AODCaloJetAvfVz_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexTotalChiSquared",          &orig_AODCaloJetAvfVertexTotalChiSquared_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexDegreesOfFreedom",         &orig_AODCaloJetAvfVertexDegreesOfFreedom_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexChi2NDoF",                 &orig_AODCaloJetAvfVertexChi2NDoF_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexDistanceToBeam",           &orig_AODCaloJetAvfVertexDistanceToBeam_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexTransverseError",          &orig_AODCaloJetAvfVertexTransverseError_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexTransverseSig",            &orig_AODCaloJetAvfVertexTransverseSig_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexDeltaEta",                 &orig_AODCaloJetAvfVertexDeltaEta_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexDeltaPhi",                 &orig_AODCaloJetAvfVertexDeltaPhi_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexRecoilPt",                 &orig_AODCaloJetAvfVertexRecoilPt_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexTrackMass",                &orig_AODCaloJetAvfVertexTrackMass_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexTrackEnergy",              &orig_AODCaloJetAvfVertexTrackEnergy_);
      aod_tree->SetBranchAddress("AODCaloJetAvfBeamSpotDeltaPhi",               &orig_AODCaloJetAvfBeamSpotDeltaPhi_);
      aod_tree->SetBranchAddress("AODCaloJetAvfBeamSpotRecoilPt",               &orig_AODCaloJetAvfBeamSpotRecoilPt_);
      aod_tree->SetBranchAddress("AODCaloJetAvfBeamSpotMedianDeltaPhi",         &orig_AODCaloJetAvfBeamSpotMedianDeltaPhi_);
      aod_tree->SetBranchAddress("AODCaloJetAvfBeamSpotLog10MedianDeltaPhi",    &orig_AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_);
      aod_tree->SetBranchAddress("AODCaloJetNCleanMatchedTracks",               &orig_AODCaloJetNCleanMatchedTracks_);
      aod_tree->SetBranchAddress("AODCaloJetSumHitsInFrontOfVert",              &orig_AODCaloJetSumHitsInFrontOfVert_);
      aod_tree->SetBranchAddress("AODCaloJetSumMissHitsAfterVert",              &orig_AODCaloJetSumMissHitsAfterVert_);
      aod_tree->SetBranchAddress("AODCaloJetHitsInFrontOfVertPerTrack",         &orig_AODCaloJetHitsInFrontOfVertPerTrack_);
      aod_tree->SetBranchAddress("AODCaloJetMissHitsAfterVertPerTrack",         &orig_AODCaloJetMissHitsAfterVertPerTrack_);
      aod_tree->SetBranchAddress("AODCaloJetAvfDistToPV",                       &orig_AODCaloJetAvfDistToPV_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexDeltaZtoPV",               &orig_AODCaloJetAvfVertexDeltaZtoPV_);
      aod_tree->SetBranchAddress("AODCaloJetAvfVertexDeltaZtoPV2",              &orig_AODCaloJetAvfVertexDeltaZtoPV2_);

      aod_tree->GetEntry(event_to_get);
      
      //cout << "orig_AODnCaloJet_ " << orig_AODnCaloJet_ << endl;

      matched=                                   true;
      AODRun_=                                   aod_run;
      AODEvent_=                                 aod_event;
      AODllpId_=                                 *orig_AODllpId_;
      AODllpStatus_=                             *orig_AODllpStatus_;
      AODllpPt_=                                 *orig_AODllpPt_;
      AODllpEta_=                                *orig_AODllpEta_;
      AODllpPhi_=                                *orig_AODllpPhi_;
      AODllpMass_=                               *orig_AODllpMass_;
      AODllpDaughterId_=                         *orig_AODllpDaughterId_;
      AODllpDaughterStatus_=                     *orig_AODllpDaughterStatus_;
      AODllpDaughterPt_=                         *orig_AODllpDaughterPt_;
      AODllpDaughterEta_=                        *orig_AODllpDaughterEta_;
      AODllpDaughterPhi_=                        *orig_AODllpDaughterPhi_;
      AODllpDaughterMass_=                       *orig_AODllpDaughterMass_;
      AODnCaloJet_=                              orig_AODnCaloJet_; 
      AODCaloJetPt_=                             *orig_AODCaloJetPt_;
      AODCaloJetEta_=                            *orig_AODCaloJetEta_;
      AODCaloJetPhi_=                            *orig_AODCaloJetPhi_;
      AODCaloJetAlphaMax_=                       *orig_AODCaloJetAlphaMax_;
      AODCaloJetAlphaMax2_=                      *orig_AODCaloJetAlphaMax2_;
      AODCaloJetAlphaMaxPrime_=                  *orig_AODCaloJetAlphaMaxPrime_;
      AODCaloJetAlphaMaxPrime2_=                 *orig_AODCaloJetAlphaMaxPrime2_;
      AODCaloJetBeta_=                           *orig_AODCaloJetBeta_;
      AODCaloJetBeta2_=                          *orig_AODCaloJetBeta2_;
      AODCaloJetSumIP_=                          *orig_AODCaloJetSumIP_;
      AODCaloJetSumIPSig_=                       *orig_AODCaloJetSumIPSig_;
      AODCaloJetMedianIP_=                       *orig_AODCaloJetMedianIP_;
      AODCaloJetMedianLog10IPSig_=               *orig_AODCaloJetMedianLog10IPSig_;
      AODCaloJetTrackAngle_=                     *orig_AODCaloJetTrackAngle_;
      AODCaloJetLogTrackAngle_=                  *orig_AODCaloJetLogTrackAngle_;
      AODCaloJetMedianLog10TrackAngle_=          *orig_AODCaloJetMedianLog10TrackAngle_;
      AODCaloJetTotalTrackAngle_=                *orig_AODCaloJetTotalTrackAngle_;
      AODCaloJetAvfVx_=                          *orig_AODCaloJetAvfVx_;
      AODCaloJetAvfVy_=                          *orig_AODCaloJetAvfVy_;
      AODCaloJetAvfVz_=                          *orig_AODCaloJetAvfVz_;
      AODCaloJetAvfVertexTotalChiSquared_=       *orig_AODCaloJetAvfVertexTotalChiSquared_;
      AODCaloJetAvfVertexDegreesOfFreedom_=      *orig_AODCaloJetAvfVertexDegreesOfFreedom_;
      AODCaloJetAvfVertexChi2NDoF_=              *orig_AODCaloJetAvfVertexChi2NDoF_;
      AODCaloJetAvfVertexDistanceToBeam_=        *orig_AODCaloJetAvfVertexDistanceToBeam_;
      AODCaloJetAvfVertexTransverseError_=       *orig_AODCaloJetAvfVertexTransverseError_;
      AODCaloJetAvfVertexTransverseSig_=         *orig_AODCaloJetAvfVertexTransverseSig_;
      AODCaloJetAvfVertexDeltaEta_=              *orig_AODCaloJetAvfVertexDeltaEta_;
      AODCaloJetAvfVertexDeltaPhi_=              *orig_AODCaloJetAvfVertexDeltaPhi_;
      AODCaloJetAvfVertexRecoilPt_=              *orig_AODCaloJetAvfVertexRecoilPt_;
      AODCaloJetAvfVertexTrackMass_=             *orig_AODCaloJetAvfVertexTrackMass_;
      AODCaloJetAvfVertexTrackEnergy_=           *orig_AODCaloJetAvfVertexTrackEnergy_;
      AODCaloJetAvfBeamSpotDeltaPhi_=            *orig_AODCaloJetAvfBeamSpotDeltaPhi_;
      AODCaloJetAvfBeamSpotRecoilPt_=            *orig_AODCaloJetAvfBeamSpotRecoilPt_;
      AODCaloJetAvfBeamSpotMedianDeltaPhi_=      *orig_AODCaloJetAvfBeamSpotMedianDeltaPhi_;
      AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_= *orig_AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_;
      AODCaloJetNCleanMatchedTracks_=            *orig_AODCaloJetNCleanMatchedTracks_;
      AODCaloJetSumHitsInFrontOfVert_=           *orig_AODCaloJetSumHitsInFrontOfVert_;
      AODCaloJetSumMissHitsAfterVert_=           *orig_AODCaloJetSumMissHitsAfterVert_;
      AODCaloJetHitsInFrontOfVertPerTrack_=      *orig_AODCaloJetHitsInFrontOfVertPerTrack_;
      AODCaloJetMissHitsAfterVertPerTrack_=      *orig_AODCaloJetMissHitsAfterVertPerTrack_;
      AODCaloJetAvfDistToPV_=                    *orig_AODCaloJetAvfDistToPV_;
      AODCaloJetAvfVertexDeltaZtoPV_=            *orig_AODCaloJetAvfVertexDeltaZtoPV_;
      AODCaloJetAvfVertexDeltaZtoPV2_=           *orig_AODCaloJetAvfVertexDeltaZtoPV2_;
      
      return 1;
    }    
  }
  return 0;
}



void merger(TString miniaod_file_name, TString aod_list_file_name){

  cout << "Running merger with arguments: " << endl;
  cout << "miniaod_file_name: " << miniaod_file_name << endl;
  cout << "aod_list_file_name: " << aod_list_file_name << endl;
  cout << "aod_event_start_offset: " << aod_event_start_offset << endl;
  
  //////////////////////////////
  // List of AOD files
  //////////////////////////////
  std::vector<TString> aod_list;
  std::ifstream input(aod_list_file_name, std::ifstream::in);
  TString aod_file_name;
  while(input>>aod_file_name){
    //aod_file_name.Remove(0, 10);
    aod_list.push_back(aod_file_name);
  }
  cout << "AOD file list size: " << aod_list.size() << endl;

  unsigned int start_suggest = 0;

  //////////////////////////////
  // MiniAOD
  //////////////////////////////

  //Formatting
  //miniaod_file_name.Remove(0, 10);
  miniaod_file_name = "root://cmseos.fnal.gov://" + miniaod_file_name;

  TFile *miniaod_file = TFile::Open(miniaod_file_name, "READ");
  if(miniaod_file->IsZombie()){
    cout << "miniaod_file is zombie" << endl;
    return;
  }
  TH1F* hEvents = (TH1F*)miniaod_file->Get("lldjNtuple/hEvents");
  TTree* miniaod_tree = (TTree*)miniaod_file->Get("lldjNtuple/EventTree");
  miniaod_tree->SetName("MINIAOD_TREE");
  //TDirectory *dir = miniaod_file->GetDirectory("lldjNtuple");

  /////////////////////////////
  // Merged output
  /////////////////////////////
  TString outfile_name = "merged_";
  outfile_name = outfile_name + miniaod_file_name.Remove(0,miniaod_file_name.Last('/')+1);
  //outfile_name = outfile_name + aod_list_file_name + miniaod_file_name.Remove(0,miniaod_file_name.Last('/')+1);
  TFile *merged_file = TFile::Open(outfile_name, "RECREATE");
  TTree *merged_tree = miniaod_tree->CloneTree();
  merged_tree->SetName("MergedEventTree");

  int merged_run;
  Long64_t merged_event;
  merged_tree->SetBranchAddress("run", &merged_run);
  merged_tree->SetBranchAddress("event", &merged_event);

  //New branches for merged tree
  TBranch* b_0 = merged_tree->Branch("AOD_matched",                                           &matched);
  TBranch* b_1 = merged_tree->Branch("AOD_run",                                               &AODRun_);
  TBranch* b_2 = merged_tree->Branch("AOD_event",                                             &AODEvent_);
  TBranch* b_3 = merged_tree->Branch("AOD_llpId",    "vector<int>",                           &AODllpId_);
  TBranch* b_4 = merged_tree->Branch("AOD_llpStatus","vector<int>",                           &AODllpStatus_);
  TBranch* b_5 = merged_tree->Branch("AOD_llpPt",    "vector<float>",                         &AODllpPt_);
  TBranch* b_6 = merged_tree->Branch("AOD_llpEta",   "vector<float>",                         &AODllpEta_);
  TBranch* b_7 = merged_tree->Branch("AOD_llpPhi",   "vector<float>",                         &AODllpPhi_);
  TBranch* b_8 = merged_tree->Branch("AOD_llpMass",   "vector<float>",                        &AODllpMass_);
  TBranch* b_9 = merged_tree->Branch("AOD_llpDaughterId",   "vector<int>",                    &AODllpDaughterId_);
  TBranch* b_10 = merged_tree->Branch("AOD_llpDaughterStatus",  "vector<int>",                 &AODllpDaughterStatus_);
  TBranch* b_11 = merged_tree->Branch("AOD_llpDaughterPt",   "vector<float>",                  &AODllpDaughterPt_);
  TBranch* b_12 = merged_tree->Branch("AOD_llpDaughterEta",  "vector<float>",                  &AODllpDaughterEta_);
  TBranch* b_13 = merged_tree->Branch("AOD_llpDaughterPhi",  "vector<float>",                  &AODllpDaughterPhi_);
  TBranch* b_14 = merged_tree->Branch("AOD_AODllpDaughterMass",   "vector<float>",             &AODllpDaughterMass_);
  TBranch* b_15 = merged_tree->Branch("AOD_AODnCaloJet",                                       &AODnCaloJet_);
  TBranch* b_16 = merged_tree->Branch("AOD_AODCaloJetPt",  "vector<float>",                    &AODCaloJetPt_);
  TBranch* b_17 = merged_tree->Branch("AOD_AODCaloJetEta", "vector<float>",                    &AODCaloJetEta_);
  TBranch* b_18 = merged_tree->Branch("AOD_AODCaloJetPhi", "vector<float>",                    &AODCaloJetPhi_);
  TBranch* b_19 = merged_tree->Branch("AOD_AODCaloJetAlphaMax", "vector<float>",               &AODCaloJetAlphaMax_);
  TBranch* b_20 = merged_tree->Branch("AOD_AODCaloJetAlphaMax2", "vector<float>",              &AODCaloJetAlphaMax2_);                               
  TBranch* b_21 = merged_tree->Branch("AOD_AODCaloJetAlphaMaxPrime", "vector<float>",          &AODCaloJetAlphaMaxPrime_);                               
  TBranch* b_22 = merged_tree->Branch("AOD_AODCaloJetAlphaMaxPrime2", "vector<float>",         &AODCaloJetAlphaMaxPrime2_);                               
  TBranch* b_23 = merged_tree->Branch("AOD_AODCaloJetBeta", "vector<float>",                   &AODCaloJetBeta_);                               
  TBranch* b_24 = merged_tree->Branch("AOD_AODCaloJetBeta2", "vector<float>",                 &AODCaloJetBeta2_);                               
  TBranch* b_25 = merged_tree->Branch("AOD_AODCaloJetSumIP", "vector<float>",                 &AODCaloJetSumIP_);
  TBranch* b_26 = merged_tree->Branch("AOD_AODCaloJetSumIPSig", "vector<float>",              &AODCaloJetSumIPSig_);
  TBranch* b_27 = merged_tree->Branch("AOD_AODCaloJetMedianIP", "vector<float>",              &AODCaloJetMedianIP_);
  TBranch* b_28 = merged_tree->Branch("AOD_AODCaloJetMedianLog10IPSig", "vector<float>",      &AODCaloJetMedianLog10IPSig_);
  TBranch* b_29 = merged_tree->Branch("AOD_AODCaloJetTrackAngle", "vector<float>",            &AODCaloJetTrackAngle_);
  TBranch* b_30 = merged_tree->Branch("AOD_AODCaloJetLogTrackAngle", "vector<float>",         &AODCaloJetLogTrackAngle_);
  TBranch* b_31 = merged_tree->Branch("AOD_AODCaloJetMedianLog10TrackAngle", "vector<float>", &AODCaloJetMedianLog10TrackAngle_);
  TBranch* b_32 = merged_tree->Branch("AOD_AODCaloJetTotalTrackAngle", "vector<float>",       &AODCaloJetTotalTrackAngle_);
  TBranch* b_33 = merged_tree->Branch("AOD_AODCaloJetAvfVx", "vector<float>",                     &AODCaloJetAvfVx_);
  TBranch* b_34 = merged_tree->Branch("AOD_AODCaloJetAvfVy", "vector<float>",                     &AODCaloJetAvfVy_);
  TBranch* b_35 = merged_tree->Branch("AOD_AODCaloJetAvfVz", "vector<float>",                     &AODCaloJetAvfVz_);
  TBranch* b_36 = merged_tree->Branch("AOD_AODCaloJetAvfVertexTotalChiSquared", "vector<float>",  &AODCaloJetAvfVertexTotalChiSquared_);
  TBranch* b_37 = merged_tree->Branch("AOD_AODCaloJetAvfVertexDegreesOfFreedom", "vector<float>", &AODCaloJetAvfVertexDegreesOfFreedom_);
  TBranch* b_38 = merged_tree->Branch("AOD_AODCaloJetAvfVertexChi2NDoF", "vector<float>",         &AODCaloJetAvfVertexChi2NDoF_);
  TBranch* b_39 = merged_tree->Branch("AOD_AODCaloJetAvfVertexDistanceToBeam", "vector<float>",   &AODCaloJetAvfVertexDistanceToBeam_);
  TBranch* b_40 = merged_tree->Branch("AOD_AODCaloJetAvfVertexTransverseError", "vector<float>",  &AODCaloJetAvfVertexTransverseError_);
  TBranch* b_41 = merged_tree->Branch("AOD_AODCaloJetAvfVertexTransverseSig", "vector<float>",    &AODCaloJetAvfVertexTransverseSig_);
  TBranch* b_42 = merged_tree->Branch("AOD_AODCaloJetAvfVertexDeltaEta", "vector<float>",         &AODCaloJetAvfVertexDeltaEta_);
  TBranch* b_43 = merged_tree->Branch("AOD_AODCaloJetAvfVertexDeltaPhi", "vector<float>",         &AODCaloJetAvfVertexDeltaPhi_);
  TBranch* b_44 = merged_tree->Branch("AOD_AODCaloJetAvfVertexRecoilPt", "vector<float>",         &AODCaloJetAvfVertexRecoilPt_);
  TBranch* b_45 = merged_tree->Branch("AOD_AODCaloJetAvfVertexTrackMass", "vector<float>",        &AODCaloJetAvfVertexTrackMass_);
  TBranch* b_46 = merged_tree->Branch("AOD_AODCaloJetAvfVertexTrackEnergy", "vector<float>",      &AODCaloJetAvfVertexTrackEnergy_);
  TBranch* b_47 = merged_tree->Branch("AOD_AODCaloJetAvfBeamSpotDeltaPhi", "vector<float>",       &AODCaloJetAvfBeamSpotDeltaPhi_);
  TBranch* b_48 = merged_tree->Branch("AOD_AODCaloJetAvfBeamSpotRecoilPt", "vector<float>",       &AODCaloJetAvfBeamSpotRecoilPt_);
  TBranch* b_49 = merged_tree->Branch("AOD_AODCaloJetAvfBeamSpotMedianDeltaPhi", "vector<float>",       &AODCaloJetAvfBeamSpotMedianDeltaPhi_);
  TBranch* b_50 = merged_tree->Branch("AOD_AODCaloJetAvfBeamSpotLog10MedianDeltaPhi", "vector<float>",  &AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_);
  TBranch* b_51 = merged_tree->Branch("AOD_AODCaloJetNCleanMatchedTracks", "vector<int>",               &AODCaloJetNCleanMatchedTracks_);
  TBranch* b_52 = merged_tree->Branch("AOD_AODCaloJetSumHitsInFrontOfVert", "vector<int>",              &AODCaloJetSumHitsInFrontOfVert_);
  TBranch* b_53 = merged_tree->Branch("AOD_AODCaloJetSumMissHitsAfterVert", "vector<int>",              &AODCaloJetSumMissHitsAfterVert_);
  TBranch* b_54 = merged_tree->Branch("AOD_AODCaloJetHitsInFrontOfVertPerTrack", "vector<int>",         &AODCaloJetHitsInFrontOfVertPerTrack_);
  TBranch* b_55 = merged_tree->Branch("AOD_AODCaloJetMissHitsAfterVertPerTrack", "vector<int>",         &AODCaloJetMissHitsAfterVertPerTrack_);
  TBranch* b_56 = merged_tree->Branch("AOD_AODCaloJetAvfDistToPV", "vector<float>",                     &AODCaloJetAvfDistToPV_);
  TBranch* b_57 = merged_tree->Branch("AOD_AODCaloJetAvfVertexDeltaZtoPV", "vector<float>",             &AODCaloJetAvfVertexDeltaZtoPV_);
  TBranch* b_58 = merged_tree->Branch("AOD_AODCaloJetAvfVertexDeltaZtoPV2", "vector<float>",            &AODCaloJetAvfVertexDeltaZtoPV2_);
   

  /////////////////////////////
  // Loop over merged tree
  /////////////////////////////
  Long64_t nentries = merged_tree->GetEntries();
  for(Long64_t i = 0; i < nentries; i++) {
    if(i%1000==0) cout << i << "/" << nentries << endl;

    ntotal++;
    merged_tree->GetEntry(i);
    
    //Clear new branches for merged tree
    matched=false;
    AODRun_=-1;
    AODEvent_=-1;
    AODllpId_.clear();
    AODllpStatus_.clear();
    AODllpPt_.clear();
    AODllpEta_.clear();
    AODllpPhi_.clear();
    AODllpMass_.clear();
    AODllpDaughterId_.clear();
    AODllpDaughterStatus_.clear();
    AODllpDaughterPt_.clear();
    AODllpDaughterEta_.clear();
    AODllpDaughterPhi_.clear();
    AODllpDaughterMass_.clear();
    AODnCaloJet_=-1;
    AODCaloJetPt_.clear();
    AODCaloJetEta_.clear();
    AODCaloJetPhi_.clear();
    AODCaloJetAlphaMax_.clear();                               
    AODCaloJetAlphaMax2_.clear();                               
    AODCaloJetAlphaMaxPrime_.clear();                               
    AODCaloJetAlphaMaxPrime2_.clear();                               
    AODCaloJetBeta_.clear();                               
    AODCaloJetBeta2_.clear();                               
    AODCaloJetSumIP_.clear();
    AODCaloJetSumIPSig_.clear();
    AODCaloJetMedianIP_.clear();
    AODCaloJetMedianLog10IPSig_.clear();
    AODCaloJetTrackAngle_.clear();
    AODCaloJetLogTrackAngle_.clear();
    AODCaloJetMedianLog10TrackAngle_.clear();
    AODCaloJetTotalTrackAngle_.clear();
    AODCaloJetAvfVx_.clear();
    AODCaloJetAvfVy_.clear();
    AODCaloJetAvfVz_.clear();
    AODCaloJetAvfVertexTotalChiSquared_.clear();
    AODCaloJetAvfVertexDegreesOfFreedom_.clear();
    AODCaloJetAvfVertexChi2NDoF_.clear();
    AODCaloJetAvfVertexDistanceToBeam_.clear();
    AODCaloJetAvfVertexTransverseError_.clear();
    AODCaloJetAvfVertexTransverseSig_.clear();
    AODCaloJetAvfVertexDeltaEta_.clear();
    AODCaloJetAvfVertexDeltaPhi_.clear();
    AODCaloJetAvfVertexRecoilPt_.clear();
    AODCaloJetAvfVertexTrackMass_.clear();
    AODCaloJetAvfVertexTrackEnergy_.clear();
    AODCaloJetAvfBeamSpotDeltaPhi_.clear();
    AODCaloJetAvfBeamSpotRecoilPt_.clear();
    AODCaloJetAvfBeamSpotMedianDeltaPhi_.clear();
    AODCaloJetAvfBeamSpotLog10MedianDeltaPhi_.clear();
    AODCaloJetNCleanMatchedTracks_.clear();
    AODCaloJetSumHitsInFrontOfVert_.clear();
    AODCaloJetSumMissHitsAfterVert_.clear();
    AODCaloJetHitsInFrontOfVertPerTrack_.clear();
    AODCaloJetMissHitsAfterVertPerTrack_.clear();
    AODCaloJetAvfDistToPV_.clear();
    AODCaloJetAvfVertexDeltaZtoPV_.clear();
    AODCaloJetAvfVertexDeltaZtoPV2_.clear();

    if(verbose) cout << "About to search for AOD matching entry " << i << endl;

    //Find aod
    int success = 0;
    if(max_events<0 || i<max_events) success = find_aod(aod_list, start_suggest, merged_run, merged_event);

    if(success==1){nmatched++;}
    if(success==0){nunmatched++;}
    
    if(verbose) cout << "Filling after success = " << success << endl;
    b_0->Fill();
    b_1->Fill();
    b_2->Fill();
    b_3->Fill();
    b_4->Fill();
    b_5->Fill();
    b_6->Fill();
    b_7->Fill();
    b_8->Fill();
    b_9->Fill();
    b_10->Fill();
    b_11->Fill();
    b_12->Fill();
    b_13->Fill();
    b_14->Fill();
    b_15->Fill();
    b_16->Fill();
    b_17->Fill();
    b_18->Fill();
    b_19->Fill();
    b_20->Fill();
    b_21->Fill();
    b_22->Fill();
    b_23->Fill();
    b_24->Fill();
    b_25->Fill();
    b_26->Fill();
    b_27->Fill();
    b_28->Fill();
    b_29->Fill();
    b_30->Fill();
    b_31->Fill();
    b_32->Fill();
    b_33->Fill();
    b_34->Fill();
    b_35->Fill();
    b_36->Fill();
    b_37->Fill();
    b_38->Fill();
    b_39->Fill();
    b_40->Fill();
    b_41->Fill();
    b_42->Fill();
    b_43->Fill();
    b_44->Fill();

  }
  cout << "end merged tree loop" << endl;

  //miniaod_tree->SetDirectory(dir);
  //TString outfile_name = "merged_";
  //outfile_name = outfile_name + miniaod_file_name.Remove(0,miniaod_file_name.Last('/')+1);
  //TFile *merged_file = TFile::Open(outfile_name, "RECREATE");
  merged_file->cd();
  cout << "debug 1" << endl;
  hEvents->Write();
  cout << "debug 2" << endl;
  merged_tree->Write();
  cout << "debug 3" << endl;
  gDirectory->Delete("MINIAOD_TREE;*");
  cout << "debug 4" << endl;
  merged_file->Close();
  cout << "debug 5" << endl;

  printf("nmatched   %llu\n", nmatched   );
  printf("nunmatched %llu\n", nunmatched );
  printf("ntotal     %llu\n", ntotal     );

  //aod_file->Close();
  //cout << "debug 6" << endl;
  //miniaod_file->Close();
  //cout << "debug 7" << endl;
  //delete merged_file;
  //delete miniaod_file;
}
