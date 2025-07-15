#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>

#include "BIBKillerCluster.hxx"
#include "SoftBoxCluster.hxx"

DECLARE_COMPONENT(BIBKillerCluster)

BIBKillerCluster::BIBKillerCluster(const std::string& name, ISvcLocator* svcLoc) : MultiTransformer(name, svcLoc,
	 KeyValues("InputClusterCollections", {"Clusters"}),
	 KeyValues("OutputClusterCollections", {"BIBKilledClusters"})) {}

// Implement Initializer
StatusCode BIBKillerCluster::initialize() {
        // Get Histogram and Data Services
        SmartIF<ITHistSvc> histSvc;
	histSvc = serviceLocator()->service("THistSvc");

        // Make Histogram
        if (m_usePt) { m_hptCuts = new TH1F("SoftKiller pT Cuts", "pTCut;Events", 100, 0, 100); }
	else { m_hptCuts = new TH1F("SoftKiller E Cuts", "ECut;Events", 100, 0, 100); }
        (void)histSvc->regHist("/histos/all/pTCuts", m_hptCuts);

	int nLamb = 2*std::floor(m_LambdaMax.value()/m_SideLength.value())+2;
        int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
        m_gridMaxes = new TH3F("SoftBox pT Maxes", "Theta;Phi",
                                                                nLamb, 0, nLamb, nPhi, 0, nPhi,
                                                                100, 0, 100);
        (void)histSvc->regHist("/histos/all/gridMaxes", m_gridMaxes);

        return StatusCode::SUCCESS;
}

std::tuple<edm4hep::ClusterCollection> BIBKillerCluster::operator()(
		const edm4hep::ClusterCollection& clusterCollection) const{
	// Make output collection
	edm4hep::ClusterCollection outputCollection;
	outputCollection.setSubsetCollection();

	// Fill SoftKiller Grid
	int nLamb = std::floor(m_LambdaMax.value()/m_SideLength.value())+1;
	int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
	std::vector<SoftBoxCluster> grid(nPhi * nLamb);

        uint64_t collectionID = clusterCollection.getID();
	debug() << clusterCollection.size() << endmsg;

	for (int j = 0; j < clusterCollection.size(); j++)
        {
		// Calculate Location
                edm4hep::Cluster cluster = clusterCollection.at(j);

		float phi = cluster.getPhi();
		float theta = cluster.getITheta();
                float energy = cluster.getEnergy();

                int index = nLamb*std::floor(phi/m_SideLength.value()+nPhi/2)+std::floor(theta/m_SideLength.value());
		(void)grid[index].addCluster(j, energy, theta, m_usePt);
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

	// Filter out all clusters below the cut
	int count = 0;
	for (int j = 0; j < nPhi * nLamb; j++) {
                //debug() << "Number of Clusters in box " << count << ": " << grid[j].getClusters().size() << "." << endmsg;
                count++;
                if (j%nLamb==0 ) {debug()<<"\n";}
                debug() << grid[j].getMax() <<"  ";
                for (ClusterInfo info : grid[j].getClusters()) {
                        if (info.val > Cut) {
                                edm4hep::Cluster cluster = clusterCollection.at(info.index);
                                outputCollection.push_back(cluster);
                        }
                }
                m_gridMaxes->Fill(
                        j % nLamb,
                        std::floor((j-(j%nLamb)) / nLamb),
                        (grid[j].getClusters().size() > 0) ? grid[j].getMax() : 0
                );
	}
        debug() << "Number of output clusters: " << outputCollection.size() << endmsg;
	return std::make_tuple(std::move(outputCollection));
}


