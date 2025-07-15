#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>
#include <TMath.h>
#include "BIBKillerCalo.hxx"
#include "SoftBoxCalo.hxx"

DECLARE_COMPONENT(BIBKillerCalo)

BIBKillerCalo::BIBKillerCalo(const std::string& name, ISvcLocator* svcLoc) : MultiTransformer(name, svcLoc,
	 { KeyValues("InputBarrelCollections", {"Barrel"}),
           KeyValues("InputEndcapCollections", {"Endcap"}) },
	 { KeyValues("OutputBarrelCollections", {"BIBKilledBarrel"}),
           KeyValues("OutputEndcapCollections", {"BIBKilledEndcap"}) }) {}

// Implement Initializer
StatusCode BIBKillerCalo::initialize() {
        // Get Histogram and Data Services
        SmartIF<ITHistSvc> histSvc;
	histSvc = serviceLocator()->service("THistSvc");

        // Make Histogram
        if (m_usePt) { m_hptCuts = new TH1F("SoftKiller pT Cuts", "pTCut;Events", 100, 0, 100); }
	else { m_hptCuts = new TH1F("SoftKiller E Cuts", "ECut;Events", 100, 0, 100); }
        (void)histSvc->regHist("/histos/all/pTCuts", m_hptCuts);

	int nTheta = 2*std::floor(m_ThetaMax.value()/m_SideLength.value())+2;
        int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
        m_gridMaxes = new TH3F("SoftBox pT Maxes", "Theta;Phi",
                                                                nTheta, 0, nTheta, nPhi, 0, nPhi,
                                                                100, 0, 100);
        (void)histSvc->regHist("/histos/all/gridMaxes", m_gridMaxes);

        return StatusCode::SUCCESS;
}

std::tuple<edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection> BIBKillerCalo::operator()(
        const std::vector<const edm4hep::CalorimeterHitCollection*>& barrelCollections, 
        const std::vector<const edm4hep::CalorimeterHitCollection*>& endcapCollections) const {
// Make output collection
	edm4hep::CalorimeterHitCollection outputBarrel;
        edm4hep::CalorimeterHitCollection outputEndcap;
	outputBarrel.setSubsetCollection();
        outputEndcap.setSubsetCollection();
        uint32_t barrelID = 0;
        uint32_t endcapID =  0;


	// Fill SoftKiller Grid
	int nTheta = 2*std::floor((m_ThetaMax.value()-m_ThetaMin.value())/m_SideLength.value())+2;
        float thetaGap = 2*(m_ThetaCenter.value()-m_ThetaMax.value());
	int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
	std::vector<SoftBoxCalo> grid(nPhi * nTheta);

        std::vector<const edm4hep::CalorimeterHitCollection*> combinedCollections;
        if (barrelCollections.size() >0) {
                combinedCollections.push_back(barrelCollections[0]);
                barrelID = barrelCollections[0]->getID();
        }
        if (endcapCollections.size() >0) {
                combinedCollections.push_back(endcapCollections[0]);
                endcapID = endcapCollections[0]->getID();
        }

        for (auto& collection : combinedCollections) {
                debug() << "Input Collection Size: " << collection->size() << endmsg;
                uint32_t collectionID = collection->getID();

                float maxTheta = 0;
                float minTheta = 3;
                for (int j = 0; j < collection->size(); j++)
                {
                        // Calculate Location
                        edm4hep::CalorimeterHit caloHit = collection->at(j);
                        edm4hep::Vector3f pos = caloHit.getPosition();
                        float phiQuo = pos.x / TMath::Sqrt(pos.x*pos.x+pos.y*pos.y);
                        float phi = (phiQuo == 1.) ? 0 : ((phiQuo == -1.) ? TMath::Pi() : TMath::ACos(phiQuo));
                        phi *= (pos.y < 0) ? -1 : 1;
                        float thetaQuo = TMath::Sqrt(pos.x*pos.x+pos.y*pos.y) / TMath::Sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z);
                        thetaQuo *= (pos.z < 0) ? -1 : 1;
                        float theta = (thetaQuo == 1.) ? 0 : ((thetaQuo == -1.) ? TMath::Pi() : TMath::ACos(thetaQuo));
                        float energy = caloHit.getEnergy();

                        float iTheta = (theta < m_ThetaCenter.value()) ? theta - m_ThetaMin.value() : theta - thetaGap;
                        int index = nTheta*std::floor(phi/m_SideLength.value()+nPhi/2)+std::floor(iTheta/m_SideLength.value());
                        if (index > nPhi*nTheta || index < 0) {error()<<"Particle at (" <<theta<<", "<<phi<<") out of range."<<endmsg; continue;}

                        if (theta > maxTheta&&theta<TMath::Pi()/2) {maxTheta = theta;}
                        if (theta < minTheta&&theta<TMath::Pi()/2) {minTheta = theta;}

                        (void)grid[index].addCalo(collectionID, j, energy, theta, m_usePt);
                }

                debug() << "\nMax Theta: "<<maxTheta<<"\nMin Theta: "<<minTheta<<endmsg;
        }

        // Find median pT of the SoftBoxes
        float Cut = 0;
        std::vector<float> values;
        // Extract all pT values from SoftBoxes into a vector
        for (int i = 0; i < nPhi * nTheta; ++i) {
                values.push_back(grid[i].getMax());
        }
        // Sort the pT values
        std::sort(values.begin(), values.end());

        // Get the median
        int n = values.size();
        float pos = m_fillPercent * (n - 1); // position in array 
        int lower = static_cast<int>(std::floor(pos));
        int upper = static_cast<int>(std::ceil(pos));
        float weight = pos - lower;
                    
        // Interpolate between the lower and upper values
        Cut = values[lower] * (1.0f - weight) + values[upper] * weight;
                
        m_hptCuts->Fill(Cut);
        debug() << "Cut: " << Cut << endmsg;

        // Filter out all clusters below the cut
        int count = 0;
        for (int j = 0; j < nPhi * nTheta; j++) {
                //debug() << "Number of Clusters in box " << count << ": " << grid[j].getClusters().size() << "." << endmsg;
                count++;
                //if (j%nTheta==0 ) {debug()<<"\n";}
                //debug() << grid[j].getMax() <<"  ";
                for (CaloInfo info : grid[j].getCalos()) {
                        if (info.val > Cut) {
                                if (info.col == barrelID) {
                                        edm4hep::CalorimeterHit caloHit = barrelCollections[0]->at(info.index);
                                        outputBarrel.push_back(caloHit);
                                } else if (info.col == endcapID) {
                                        edm4hep::CalorimeterHit caloHit = endcapCollections[0]->at(info.index);
                                        outputEndcap.push_back(caloHit);
                                } else {debug() << info.col << "   " << endcapID << ", " << barrelID << endmsg;}
                                
                        }
                }
                m_gridMaxes->Fill(
                        j % nTheta,
                        std::floor((j-(j%nTheta)) / nTheta),
                        (grid[j].getCalos().size() > 0) ? grid[j].getMax() : 0
                );
        }
        debug() << "Number of output barrel calo hits: " << outputBarrel.size() << endmsg;
        debug() << "Number of output endcap calo hits: " << outputEndcap.size() << endmsg;
	return std::make_tuple(std::move(outputBarrel), std::move(outputEndcap));
}


