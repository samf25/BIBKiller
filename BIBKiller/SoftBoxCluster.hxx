#ifndef SoftBoxCluster_h
#define SoftBoxCluster_h 1

#include <edm4hep/Cluster.h>
#include <vector>
#include <utility>
#include <tuple>

class SoftBoxCluster {
public:
	SoftBoxCluster();  // Constructor declaration
	std::tuple<float, float> addCluster(const edm4hep::Cluster& cluster, float Bz, bool usePt);  // Method to add a cluster
	float getMax() const;  // Method to get the maximum value
	std::vector<std::pair<const edm4hep::Cluster, float>> getClusters() const; // Method to retrieve all cluster and Vals
private:
	std::vector<std::pair<const edm4hep::Cluster, float>> clusterValPairs;  // Vector to hold pointers to Cluster
	float maxVal;  // Variable to track the maximum value
};

#endif // SOFTBOXCLUSTER_H

