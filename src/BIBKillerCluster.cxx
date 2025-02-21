#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>

#include "BIBKillerCluster.hxx"
#include "SoftBoxCluster.hxx"

DECLARE_COMPONENT(BIBKillerCluster)

BIBKillerCluster::BIBKillerCluster(const std::string& name, ISvcLocator* svcLoc) : Transformer(name, svcLoc,
	 KeyValues("InputClusterCollectionName", {"Clusters"}),
	 KeyValues("OutputClusterCollectionName", {"BIBKilledClusters"})) {}

// Implement Initializer
StatusCode BIBKillerCluster::initialize() {
        // Get Histogram and Data Services
        ITHistSvc* histSvc{nullptr};
        StatusCode sc1 = service("THistSvc", histSvc);
        if ( sc1.isFailure() ) {
                error() << "Could not locate HistSvc" << endmsg;
                return StatusCode::FAILURE; }

        // Make Histogram
        if (m_usePt) { m_hptCuts = new TH1F("SoftKiller pT Cuts", "pTCut;Events", 100, 0, 100); }
	else { m_hptCuts = new TH1F("SoftKiller E Cuts", "ECut;Events", 100, 0, 100); }
        (void)histSvc->regHist("/histos/all/pTCuts", m_hptCuts);

        return StatusCode::SUCCESS;
}

edm4hep::ClusterCollection BIBKillerCluster::operator()(
		const edm4hep::ClusterCollection& clusterCollection) const{
	MsgStream log(msgSvc(), name());

	// Make output collection
	edm4hep::ClusterCollection outputClusters;
	outputClusters.setSubsetCollection();
	// Fill SoftKiller Grid
	int nLamb = 2*std::floor(m_LambdaMax.value()/m_SideLength.value())+2;
	int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
	SoftBoxCluster grid[nPhi * nLamb];
	for (const auto& cluster : clusterCollection) {
		// Calculate Location
		float phi = cluster.getPhi();
		float theta = cluster.getITheta();
		log << MSG::DEBUG << "\nPhi: " << phi << "\nTheta: " << theta<< endmsg;
		int index = nLamb*std::floor(phi/m_SideLength.value()+nPhi/2)+std::floor(theta/m_SideLength.value()+nLamb/2);
		(void)grid[index].addTrack(cluster, m_Bz, m_usePt);
	}	

	// Find median pT of the SoftBoxes
	float Cut = 0;
	std::vector<float> values;
        // Extract all pT values from SoftBoxes into a vector
        for (int i = 0; i < nPhi * nLamb; ++i) {
                values.push_back(grid[i].getMax());
        }
        // Sort the pT values
        std::sort(values.begin(), values.end());

        // Get the median
        int n = values.size();
        if (n % 2 == 0) {
                // If even, average the two middle values
                Cut = (values[n/2 - 1] + values[n/2]) / 2.0f;
        } else {
                // If odd, the middle value
                Cut = values[n/2];
        }	
	
	m_hptCuts->Fill(Cut);

	// Filter out all tracks below the pT cut
	int count = 0;
	for (SoftBox box : grid) {
		log << MSG::DEBUG << "Number of Clusters in box " << count << ": " << box.getClusters().size() << "." << endmsg;
		count++;
		for (std::pair<const edm4hep::Cluster, float> pair : box.getClusters()) {
			if (pair.second > ptCut) {
				outputClusters.push_back(pair.first);
			}
		}
	}

	return outputClusters;
}

