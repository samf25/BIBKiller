#ifndef MergeClusterCollections_h
#define MergeClusterCollections_h 1

// edm4hep
#include <edm4hep/ClusterCollection.h>

// k4FWCore
#include <k4FWCore/DataHandle.h>
#include <k4FWCore/Transformer.h>

// Standard
#include <vector>

/**
 * @brief Combine Cluster collections into collection
 * @author Samuel Ferraro
 * @version $Id$
 */
struct MergeClusterCollections final : k4FWCore::Transformer<edm4hep::ClusterCollection(const std::vector<const edm4hep::ClusterCollection*>&)> {
public:
	/**
         * @brief Constructor for ACTSMergeHitCollections
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	MergeClusterCollections(const std::string& name, ISvcLocator* svcLoc);

	/**
         * @brief MergeClusterCollection operation. The workhorse of this MultiTransformer.
         * @param cols A vector collection of clusters from one section of the detector
         * @return A merged collection with all tracker hits.
         */
	edm4hep::ClusterCollection operator()(
		const std::vector<const edm4hep::ClusterCollection*>& cols) const override;
};
#endif // MergeClusterCollection_h
