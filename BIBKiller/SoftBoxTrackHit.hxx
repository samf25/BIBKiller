#ifndef SoftBoxTrackHit_h
#define SoftBoxTrackHit_h 1

#include <vector>
#include <utility>
#include <tuple>

struct TrackHitInfo {
    int index;
	uint32_t col;
    float val;
};

class SoftBoxTrackHit {
public:
	SoftBoxTrackHit();  // Constructor declaration
	std::tuple<float, float> addTrackHit(uint32_t col, int index, float energy, float theta, bool usePt);  // Method to add a TrackHit
	float getMax() const;  // Method to get the maximum value
	std::vector<TrackHitInfo> getTrackHits() const; // Method to retrieve all TrackHit and Vals
private:
	std::vector<TrackHitInfo> trackHitValPairs;  // Vector to hold pointers to TrackHit 
	float maxVal;  // Variable to track the maximum value
};

#endif // SOFTBOXTRACKHIT_H

