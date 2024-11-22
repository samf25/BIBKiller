#include <edm4hep/TrackState.h>

#include <podio/ROOTWriter.h>
#include <podio/EventStore.h>
#include <edm4hep/MutableTrack.h>
#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>

#include "BIBKiller.hxx"
#include "SoftBox.hxx"

DECLARE_COMPONENT(BIBKiller)

BIBKiller::BIBKiller(const std::string& name, ISvcLocator* svcLoc) : Transformer(name, svcLoc,
	 KeyValue("InputTrackCollectionName", "Tracks"),
	 KeyValue("OutputTrackCollectionName", "BIBKilledTracks")) {}

// Implement Initializer
StatusCode BIBKiller::initialize() {
        // Get Histogram and Data Services
        ITHistSvc* histSvc{nullptr};
        StatusCode sc1 = service("THistSvc", histSvc);
        if ( sc1.isFailure() ) {
                error() << "Could not locate HistSvc" << endmsg;
                return StatusCode::FAILURE; }

        // Make Histogram
        m_hptCuts = new TH1F("SoftKiller pT Cuts", "pTCut;Events", 100, 0, 100);
        (void)histSvc->regHist("/histos/all/pTCuts", m_hptCuts);

        return StatusCode::SUCCESS;
}

edm4hep::TrackCollection BIBKiller::operator()(
		const edm4hep::TrackCollection& trackCollection) const{
	MsgStream log(msgSvc(), name());

	// Make output collection
	edm4hep::TrackCollection outputTracks;
	outputTracks.setSubsetCollection();
	// Fill SoftKiller Grid
	SoftBox grid[m_nPhiRows * m_nThetaCols];
	SoftBox overflow;
	for (const auto& track : trackCollection) {
		// Calculate Location
		const edm4hep::TrackState& state = track.getTrackStates(edm4hep::TrackState::AtIP);
		float phi = state.phi + TMath::Pi();
		float theta = TMath::Pi() /2 - std::atan(state.tanLambda);
		log << MSG::DEBUG << "\nPhi: " << phi << "\nTheta: " << theta << endmsg;
		// Catch Overflow
		if (fabs(theta) > m_ThetaMax || fabs(phi) > m_PhiMax) {
			(void)overflow.addTrack(track, m_Bz);
		} else {
			(void)grid[std::min(std::max(int(std::floor(phi/m_PhiMax*m_nPhiRows)), 0), m_nPhiRows -1) + std::min(int(std::floor(theta/m_ThetaMax*m_nThetaCols)), m_nThetaCols -1)*(m_nPhiRows)].addTrack(track, m_Bz);
		}
	}	

	// Find median pT of the SoftBoxes
	float ptCut = 0;
	std::vector<float> ptValues;
        // Extract all pT values from SoftBoxes into a vector
        for (int i = 0; i < m_nPhiRows * m_nThetaCols; ++i) {
                ptValues.push_back(grid[i].getMaxPt());
		log << MSG::WARNING << grid[i].getMaxPt() << endmsg;
        }
        // Sort the pT values
        std::sort(ptValues.begin(), ptValues.end());

        // Get the median
        int n = ptValues.size();
        if (n % 2 == 0) {
                // If even, average the two middle values
                ptCut = (ptValues[n/2 - 1] + ptValues[n/2]) / 2.0f;
        } else {
                // If odd, the middle value
                ptCut = ptValues[n/2];
        }	
	
	m_hptCuts->Fill(ptCut);
	
	// Filter out all tracks below the pT cut
	for (SoftBox box : grid) {
		for (std::pair<const edm4hep::Track, float> pair : box.getTracks()) {
			if (pair.second > ptCut) {
				log << MSG::DEBUG << pair.first.id() <<endmsg;
				outputTracks.push_back(pair.first);
			}
		}
	}

	// Add overflow
	log << MSG::DEBUG << "Number of Tracks in overflow: " << overflow.getTracks().size() << "." << endmsg;
	if (m_KeepOverflow) {
		for (std::pair<const edm4hep::Track, float> pair : overflow.getTracks()) {
			if (pair.second > ptCut) {
				outputTracks.push_back(pair.first);
			}
		}
	}

	return outputTracks;
}

