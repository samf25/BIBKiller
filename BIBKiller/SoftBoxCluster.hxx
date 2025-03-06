#ifndef SoftBoxCluster_h
#define SoftBoxCluster_h 1

#include <edm4hep/Cluster.h>
#include <vector>
#include <utility>
#include <tuple>

struct ClusterInfo {
    edm4hep::Cluster cluster;
    float val;
    int collection;
};

class SoftBoxCluster {
public:
	SoftBoxCluster();  // Constructor declaration
	std::tuple<float, float> addCluster(const edm4hep::Cluster& cluster, float Bz, bool usePt, int collection);  // Method to add a cluster
	float getMax() const;  // Method to get the maximum value
	std::vector<ClusterInfo> getClusters() const; // Method to retrieve all cluster and Vals
private:
	std::vector<ClusterInfo> clusterValPairs;  // Vector to hold pointers to Cluster
	float maxVal;  // Variable to track the maximum value
};

#endif // SOFTBOXCLUSTER_H

