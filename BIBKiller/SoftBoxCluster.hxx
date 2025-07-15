#ifndef SoftBoxCluster_h
#define SoftBoxCluster_h 1

#include <edm4hep/Cluster.h>
#include <vector>
#include <utility>
#include <tuple>

struct ClusterInfo {
    int index;
    float val;
};

class SoftBoxCluster {
public:
	SoftBoxCluster();  // Constructor declaration
	std::tuple<float, float> addCluster(int index, float energy, float theta, bool usePt);  // Method to add a cluster
	float getMax() const;  // Method to get the maximum value
	std::vector<ClusterInfo> getClusters() const; // Method to retrieve all cluster and Vals
private:
	std::vector<ClusterInfo> clusterValPairs;  // Vector to hold pointers to Cluster
	float maxVal;  // Variable to track the maximum value
};

#endif // SOFTBOXCLUSTER_H

