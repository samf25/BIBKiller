#ifndef SoftBox_h
#define SoftBox_h 1

#include <edm4hep/Track.h>
#include <vector>
#include <utility>

class SoftBox {
public:
	SoftBox();  // Constructor declaration
	void addTrack(const edm4hep::Track* track, float Bz);  // Method to add a track
	float getMaxPt() const;  // Method to get the maximum pT
	std::vector<std::pair<const edm4hep::Track*, float>> getTracks() const; // Method to retrieve all tracks and pTs

private:
	std::vector<std::pair<const edm4hep::Track*, float>> trackPtPairs;  // Vector to hold pointers to Tracks
	float maxPt;  // Variable to track the maximum pT
};

#endif // SOFTBOX_H

