#include <edm4hep/TrackState.h>
#include <podio/ROOTWriter.h>
#include <podio/EventStore.h>
#include <edm4hep/MutableTrack.h>

#include <TMath.h>

#include <utility>

#include "BIBKiller.hxx"
#include "SoftBox.hxx"

DECLARE_COMPONENT(BIBKiller)

BIBKiller::BIBKiller(const std::string& name, ISvcLocator* svcLoc) : Transformer(name, svcLoc,
	 KeyValue("InputTrackCollectionName", "Tracks"),
	 KeyValue("OutputTrackCollectionName", "BIBKilledTracks")) {}

edm4hep::TrackCollection BIBKiller::operator()(
		const edm4hep::TrackCollection& trackCollection) const{
	MsgStream log(msgSvc(), name());

	// Make output collection
	edm4hep::TrackCollection outputTracks;
	outputTracks.setSubsetCollection();
	// Fill SoftKiller Grid
	SoftBox grid[m_nPhiRows * m_nThetaCols];
	for (const auto& track : trackCollection) {
		const edm4hep::TrackState& state = track.getTrackStates(edm4hep::TrackState::AtIP);
		float phi = state.phi + TMath::Pi();
		float theta = TMath::Pi() /2 - std::atan(state.tanLambda);
		log << MSG::DEBUG << "\nPhi: " << phi << "\nTheta: " << theta << endmsg;
		grid[int(std::floor(phi/(TMath::Pi()*2)*m_nPhiRows) + std::floor(theta/(TMath::Pi())*m_nThetaCols)*(m_nPhiRows - 1))].addTrack(&track, m_Bz);
	}	

	// Find median pT of the SoftBoxes
	float ptCut = 0;
	std::vector<float> ptValues;
        // Extract all pT values from SoftBoxes into a vector
        for (int i = 0; i < m_nPhiRows * m_nThetaCols; ++i) {
                ptValues.push_back(grid[i].getMaxPt());
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

	// Filter out all tracks below the pT cut
	for (SoftBox box : grid)
		for (std::pair<const edm4hep::Track*, float> pair : box.getTracks())
			if (pair.second > ptCut)
				outputTracks.push_back(*pair.first);

	return outputTracks;
}

