#ifndef BIBKillerCalo_h
#define BIBKillerCalo_h 1

// edm4hep
#include <edm4hep/CalorimeterHitCollection.h>
#include <edm4hep/CalorimeterHit.h>

// k4FWCore
#include <k4FWCore/DataHandle.h>
#include <k4FWCore/Transformer.h>

#include <TH3.h>
#include <TH1.h>
#include <TMath.h>

#include <tuple>

#include "SoftBoxCalo.hxx"

//! \brief Apply the SoftDrop Algorithm
/**
 * Remove tracks until half the grids
 * are empty.
 *
 * @author Samuel Ferraro
 * @version $Id$
 */
struct BIBKillerCalo final : k4FWCore::MultiTransformer <std::tuple<edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection>(const std::vector<const edm4hep::CalorimeterHitCollection*>&, const std::vector<const edm4hep::CalorimeterHitCollection*>&)> {
public:
	/**
         * @brief Constructor for BIBKiller
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	BIBKillerCalo(const std::string& name, ISvcLocator* svcLoc);

	/**
 	 * @brief Register and create all histograms (and Histogram Service)
 	 */
	StatusCode initialize();
		
	/**
         * @brief BIBKiller operation. The workhorse of this Transformer.
         * @param trackCollection A collection of reconstructed tracks with BIB contamination
         * @return A Track Collection SoftKiller applied
         */
	std::tuple<edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection> operator()(const std::vector<const edm4hep::CalorimeterHitCollection*>& barrelCollections, const std::vector<const edm4hep::CalorimeterHitCollection*>& endcapCollections) const override;

protected:
	Gaudi::Property<float> m_SideLength{this, "SideLength", 0.9, "Side length of SoftKiller grid."};
	Gaudi::Property<float> m_PhiMax{this, "PhiMax", TMath::Pi(), "Maximum allowed value of phi (absolute value)."};
	Gaudi::Property<float> m_ThetaMax{this, "ThetaMax", TMath::Pi()/2, "Maximum allowed value of Theta. (Up to Pi/2, will be mirrored)"};
	Gaudi::Property<float> m_ThetaMin{this, "ThetaMin", 0, "Minimum allowed value of Theta."};
	Gaudi::Property<float> m_ThetaCenter{this, "ThetaCenter", TMath::Pi()/2, "Value Theta is mirrored over (endcaps)."};
	Gaudi::Property<float> m_fillPercent{this, "FillPercent", 0.5, "Percentage of grid boxes to make empty."};
	Gaudi::Property<bool>  m_KeepOverflow{this, "KeepOverflow", true, "Should the algorithm keep or remove all overflow."};
	Gaudi::Property<bool>  m_usePt{this, "UsePt", true, "Should the algorithm use pT or E as the variable."};

	TH3* m_gridMaxes;
	TH1* m_hptCuts;
	
	float m_Bz = 3.57;
};
#endif
