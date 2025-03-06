#include <TMath.h>

#include "SoftBoxCluster.hxx"

// Constructor initializes maxPt to 0
SoftBoxCluster::SoftBoxCluster() { maxVal = 0; }

// Method to add a Track pointer and update maxPt if needed
std::tuple<float, float> SoftBoxCluster::addCluster(const edm4hep::Cluster& cluster, float Bz, bool usePt, int collection) {
	float val = 0;
	if (usePt) { val = cluster.getEnergy() * TMath::Cos(cluster.getITheta()); }
	else { val = cluster.getEnergy(); }
	
	clusterValPairs.push_back({cluster, val, collection});
	if (val > maxVal) { maxVal = val; }

	return std::make_tuple(val, maxVal);
}

// Method to retrieve the maximum value
float SoftBoxCluster::getMax() const { return maxVal; }

// Method to retieve cluster and their values
std::vector<ClusterInfo> SoftBoxCluster::getClusters() const { return clusterValPairs; }
