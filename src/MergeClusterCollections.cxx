#include "MergeClusterCollections.hxx"

// edm4hep
#include <edm4hep/Cluster.h>


DECLARE_COMPONENT(MergeClusterCollections)


MergeClusterCollections::MergeClusterCollections(const std::string& name, ISvcLocator* svcLoc) : Transformer(name, svcLoc, {
		KeyValues("InputCollections", {"Collection"}) },
	      { KeyValues("OutputCollection", {"MergedCollection"}) }) {}

edm4hep::ClusterCollection MergeClusterCollections::operator()(
		const std::vector<const edm4hep::ClusterCollection*>& cols) const{
	// Initialize collection
	edm4hep::ClusterCollection mergedCollection;
	
	mergedCollection.setSubsetCollection();
	// Loop over all items in all collections and add each to a new collection
	for (size_t i = 0; i < cols.size(); ++i) {
		for (const auto& item : *(cols[i])) {
			mergedCollection.push_back(item);
		}
	}

	return mergedCollection;
}
