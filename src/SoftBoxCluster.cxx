#include <TMath.h>

#include "SoftBoxCluster.hxx"
#include <iostream>

// Constructor initializes maxPt to 0
SoftBoxCluster::SoftBoxCluster() { maxVal = 0; }

// Method to add a Track pointer and update maxPt if needed
std::tuple<float, float> SoftBoxCluster::addCluster(int index, float energy, float theta, bool usePt) {
	float val = 0;
	if (usePt) { val = std::abs(energy * TMath::Cos(theta)); }
	else { val = energy; }

	ClusterInfo info(index, val);
	clusterValPairs.emplace_back(info);
	if (val > maxVal) { maxVal = val; }

	return std::make_tuple(val, maxVal);
}

// Method to retrieve the maximum value
float SoftBoxCluster::getMax() const { return maxVal; }

// Method to retieve cluster and their values
std::vector<ClusterInfo> SoftBoxCluster::getClusters() const { return clusterValPairs; }
