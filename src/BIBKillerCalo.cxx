#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>
#include <TMath.h>
#include "DDSegmentation/BitFieldCoder.h"
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

	int nPhi = m_phiBoxes.value();
        int nTheta = std::ceil((m_ThetaMax.value() - m_ThetaMin.value()) / TMath::Pi() * nPhi);
        m_gridMaxes = new TH3F("SoftBox pT Maxes", "Theta;Phi",
                                                                nTheta, 0, nTheta, nPhi, 0, nPhi,
                                                                100, 0, 100);
        (void)histSvc->regHist("/histos/all/gridMaxes", m_gridMaxes);

        if (m_useUniformTheta.value() && (m_ThetaMin.value() != 0.0 || std::abs(m_ThetaMax.value() - TMath::Pi()/2.0) > 1e-5)) {
                error() << "Uniform Theta distribution is not compatible with Theta Gaps." << endmsg;                return StatusCode::FAILURE;
        }

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
        uint32_t endcapID = 0;

        dd4hep::DDSegmentation::BitFieldCoder bitFieldCoder("system:5,side:-2,module:8,stave:4,layer:9,submodule:4,x:32:-16,y:-16");

	// Set up grid parameters
	int nPhi = m_phiBoxes.value();
        int nTheta = std::ceil((m_ThetaMax.value() - m_ThetaMin.value()) / TMath::Pi() * nPhi);
        float phiSideLength = 2 * TMath::Pi() / nPhi;
        float thetaSideLength = 2 * (m_ThetaMax.value() - m_ThetaMin.value()) / nTheta;

        for (int layerIDX = 0; layerIDX < m_layerBins.value().size() -1; layerIDX++) {

                // Create grid for this layer
	        std::vector<SoftBoxCalo> grid(nPhi * nTheta);

                // Collection the collections
                std::vector<const edm4hep::CalorimeterHitCollection*> combinedCollections;
                if (barrelCollections.size() >0) {
                        combinedCollections.push_back(barrelCollections[0]);
                        barrelID = barrelCollections[0]->getID();
                }
                if (endcapCollections.size() >0) {
                        combinedCollections.push_back(endcapCollections[0]);
                        endcapID = endcapCollections[0]->getID();
                }

                // Loop over the collections
                for (auto& collection : combinedCollections) {
                        debug() << "Input Collection Size: " << collection->size() << endmsg;
                        uint32_t collectionID = collection->getID();

                        for (int j = 0; j < collection->size(); j++)
                        {

                                // Calculate Location
                                edm4hep::CalorimeterHit caloHit = collection->at(j);
                                int layer = bitFieldCoder.get(caloHit.getCellID(), "layer");
                                if (layer < m_layerBins[layerIDX] || layer >= m_layerBins[layerIDX + 1]) continue;

                                edm4hep::Vector3f pos = caloHit.getPosition();
                                // Improved phi calculation using atan2
                                float phi = std::atan2(pos.y, pos.x) + TMath::Pi();
                                float theta = TMath::ACos(pos.z / TMath::Sqrt(pos.x*pos.x+pos.y*pos.y+pos.z*pos.z));
                                float energy = caloHit.getEnergy();

                                int thetaIndex;
                                if (m_useUniformTheta.value()) {
                                        if (m_isECAL.value()) {
                                                auto it = std::upper_bound(m_ECALbinEdges[nTheta-2].begin(), m_ECALbinEdges[nTheta-2].end(), theta);
                                                thetaIndex = std::distance(m_ECALbinEdges[nTheta-2].begin(), it);
                                                if (thetaIndex > nTheta-1) {error()<<"Particle at (" <<theta<<", "<<phi<<") out of range."<<endmsg; continue;}
                                        } else {
                                                auto it = std::upper_bound(m_HCALbinEdges[nTheta-2].begin(), m_HCALbinEdges[nTheta-2].end(), theta);
                                                thetaIndex = std::distance(m_HCALbinEdges[nTheta-2].begin(), it);
                                                if (thetaIndex > nTheta-1) {error()<<"Particle at (" <<theta<<", "<<phi<<") out of range."<<endmsg; continue;}
                                        }
                                } else {
                                        if (theta < TMath::Pi()/2) {
                                                thetaIndex = std::ceil((m_ThetaMax.value() - theta) / thetaSideLength);
                                        } else {
                                                thetaIndex = std::ceil((theta - TMath::Pi() + m_ThetaMax.value()) / thetaSideLength) + nTheta/2;
                                        }
                                }

                                int phiIndex;
                                if (phi < TMath::Pi()) {
                                        phiIndex = std::ceil((TMath::Pi() - phi) / phiSideLength);
                                } else {
                                        phiIndex = std::ceil((phi - TMath::Pi()) / phiSideLength) + nPhi/2;
                                }

                                int index = nTheta * (phiIndex -1) + thetaIndex -1;
                                if (index > nPhi*nTheta || index < 0) {
                                        error() << "Particle at (" << theta << ", " << phi << ") out of range." << endmsg;
                                        error() << "Index: " << index << ", nPhi: " << nPhi << ", nTheta: " << nTheta << endmsg;
                                        error() << "PhiIndex: " << phiIndex << ", ThetaIndex: " << thetaIndex << endmsg;
                                        continue;
                                }


                                (void)grid[index].addCalo(collectionID, j, energy, theta, m_usePt);
                        }
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
                        //debug() << "Number of Clusters in box " << count << ": " << grid[j].getCalos().size() << "." << endmsg;
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
        }

        debug() << "Number of output barrel calo hits: " << outputBarrel.size() << endmsg;
        debug() << "Number of output endcap calo hits: " << outputEndcap.size() << endmsg;
	return std::make_tuple(std::move(outputBarrel), std::move(outputEndcap));
}


