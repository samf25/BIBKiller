#include <edm4hep/TrackState.h>
#include <TMath.h>

#include "SoftBox.hxx"

// Constructor initializes maxPt to 0
SoftBox::SoftBox() : maxPt(0) {}

// Method to add a Track pointer and update maxPt if needed
std::tuple<float, float> SoftBox::addTrack(const edm4hep::Track& track, float Bz) {
	// Update maxPt if this track has a higher pT
	float pt = fabs(0.3 * Bz / track.getTrackStates(edm4hep::TrackState::AtIP).omega /1000);
	if (pt > maxPt) { maxPt = pt; }
	// Add track to the vector
	trackPtPairs.push_back(std::make_pair(track, pt));
	return std::make_tuple(pt, maxPt);
}

// Method to retrieve the maximum pT
float SoftBox::getMaxPt() const { return maxPt; }

// Method to retieve tracks and their pTs
std::vector<std::pair<const edm4hep::Track, float>> SoftBox::getTracks() const { return trackPtPairs; }
