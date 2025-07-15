#include <TMath.h>

#include "SoftBoxTrackHit.hxx"
#include <iostream>

// Constructor initializes maxPt to 0
SoftBoxTrackHit::SoftBoxTrackHit() { maxVal = 0; }

// Method to add a Track pointer and update maxPt if needed
std::tuple<float, float> SoftBoxTrackHit::addTrackHit(uint32_t col, int index, float energy, float theta, bool usePt) {
	float val = 0;
	if (usePt) { val = std::abs(energy * TMath::Cos(theta)); }
	else { val = energy; }

	TrackHitInfo info(index, col, val);
	trackHitValPairs.emplace_back(info);
	if (val > maxVal) { maxVal = val; }

	return std::make_tuple(val, maxVal);
}

// Method to retrieve the maximum value
float SoftBoxTrackHit::getMax() const { return maxVal; }

// Method to retieve TrackHit and their values
std::vector<TrackHitInfo> SoftBoxTrackHit::getTrackHits() const { return trackHitValPairs; }
