#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>
#include <TMath.h>
#include "BIBKillerTrackHit.hxx"
#include "SoftBoxTrackHit.hxx"

DECLARE_COMPONENT(BIBKillerTrackHit)

BIBKillerTrackHit::BIBKillerTrackHit(const std::string& name, ISvcLocator* svcLoc) : MultiTransformer(name, svcLoc,
	 { KeyValues("InputBarrelCollections", {"Barrel"}),
           KeyValues("InputEndcapCollections", {"Endcap"}) },
	 { KeyValues("OutputBarrelCollections", {"BIBKilledBarrel"}),
           KeyValues("OutputEndcapCollections", {"BIBKilledEndcap"}) }) {}

// Implement Initializer
StatusCode BIBKillerTrackHit::initialize() {
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

std::tuple<edm4hep::TrackerHitPlaneCollection, edm4hep::TrackerHitPlaneCollection> BIBKillerTrackHit::operator()(
        const edm4hep::TrackerHitPlaneCollection& barrelCollections, 
        const edm4hep::TrackerHitPlaneCollection& endcapCollections) const {
// Make output collection
	edm4hep::TrackerHitPlaneCollection outputBarrel;
        edm4hep::TrackerHitPlaneCollection outputEndcap;
	outputBarrel.setSubsetCollection();
        outputEndcap.setSubsetCollection();
        uint32_t barrelID = barrelCollections.getID();
        uint32_t endcapID = endcapCollections.getID();


	// Fill SoftKiller Grid
	int nLamb = std::floor(m_LambdaMax.value()/m_SideLength.value())+1;
	int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
	std::vector<SoftBoxTrackHit> grid(nPhi * nLamb);

        std::vector<const edm4hep::TrackerHitPlaneCollection*> combinedCollections;
        combinedCollections.push_back(&barrelCollections);
        combinedCollections.push_back(&endcapCollections);

        for (auto& collection : combinedCollections) {
                uint32_t collectionID = collection->getID();
                debug() << "Input Collection Size: " << collection->size() << endmsg;

                for (int j = 0; j < collection->size(); j++)
                {
                        // Calculate Location
                        edm4hep::TrackerHitPlane trackHit = collection->at(j);
                        edm4hep::Vector3d pos = trackHit.getPosition();
                        float phiQuo = pos.x / TMath::Sqrt(pos.x*pos.x+pos.y*pos.y);
                        float phi = (phiQuo == 1.) ? 0 : ((phiQuo == -1.) ? TMath::Pi() : TMath::ACos(phiQuo));
                        phi *= (pos.y < 0) ? -1 : 1;
                        float thetaQuo = TMath::Sqrt(pos.x*pos.x+pos.y*pos.y) / TMath::Sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z);
                        thetaQuo *= (pos.z < 0) ? -1 : 1;
                        float theta = (thetaQuo == 1.) ? 0 : ((thetaQuo == -1.) ? TMath::Pi() : TMath::ACos(thetaQuo));
                        float energy = trackHit.getEDep();

                        int index = nLamb*std::floor(phi/m_SideLength.value()+nPhi/2)+std::floor(theta/m_SideLength.value());

                        (void)grid[index].addTrackHit(collectionID, j, energy, theta, m_usePt);
                }
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
        float pos = 0.5 * (n - 1); // position in array 
        int lower = static_cast<int>(std::floor(pos));
        int upper = static_cast<int>(std::ceil(pos));
        float weight = pos - lower;
                    
        // Interpolate between the lower and upper values
        Cut = values[lower] * (1.0f - weight) + values[upper] * weight;
                
        m_hptCuts->Fill(Cut);
        debug() << "Cut: " << Cut << endmsg;

        // Filter out all clusters below the cut
        int count = 0;
        for (int j = 0; j < nPhi * nLamb; j++) {
                //debug() << "Number of Clusters in box " << count << ": " << grid[j].getClusters().size() << "." << endmsg;
                count++;
                //if (j%nLamb==0 ) {debug()<<"\n";}
                //debug() << grid[j].getMax() <<"  ";
                for (TrackHitInfo info : grid[j].getTrackHits()) {
                        if (info.val > Cut) {
                                if (info.col == barrelID) {
                                        edm4hep::TrackerHitPlane trackHit = barrelCollections.at(info.index);
                                        outputBarrel.push_back(trackHit);
                                } else if (info.col == endcapID) {
                                        edm4hep::TrackerHitPlane trackHit = endcapCollections.at(info.index);
                                        outputEndcap.push_back(trackHit);
                                } else {debug() << info.col << "   " << endcapID << ", " << barrelID << endmsg;}
                                
                        }
                }
                m_gridMaxes->Fill(
                        j % nLamb,
                        std::floor((j-(j%nLamb)) / nLamb),
                        (grid[j].getTrackHits().size() > 0) ? grid[j].getMax() : 0
                );
        }
        debug() << "Number of output barrel TrackHits: " << outputBarrel.size() << endmsg;
        debug() << "Number of output endcap TrackHits: " << outputEndcap.size() << endmsg;
	return std::make_tuple(std::move(outputBarrel), std::move(outputEndcap));
}


