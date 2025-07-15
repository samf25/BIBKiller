#ifndef BIBKillerTrackHit_h
#define BIBKillerTrackHit_h 1

// edm4hep
#include <edm4hep/TrackerHitPlaneCollection.h>
#include <edm4hep/TrackerHitPlane.h>

// k4FWCore
#include <k4FWCore/DataHandle.h>
#include <k4FWCore/Transformer.h>

#include <TH3.h>
#include <TH1.h>
#include <TMath.h>

#include <tuple>

#include "SoftBoxTrackHit.hxx"

//! \brief Apply the SoftDrop Algorithm
/**
 * Remove tracks until half the grids
 * are empty.
 *
 * @author Samuel Ferraro
 * @version $Id$
 */
struct BIBKillerTrackHit final : k4FWCore::MultiTransformer <std::tuple<edm4hep::TrackerHitPlaneCollection, edm4hep::TrackerHitPlaneCollection>(const edm4hep::TrackerHitPlaneCollection&, const edm4hep::TrackerHitPlaneCollection&)> {
public:
	/**
         * @brief Constructor for BIBKiller
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	BIBKillerTrackHit(const std::string& name, ISvcLocator* svcLoc);

	/**
 	 * @brief Register and create all histograms (and Histogram Service)
 	 */
	StatusCode initialize();
		
	/**
         * @brief BIBKiller operation. The workhorse of this Transformer.
         * @param trackCollection A collection of reconstructed tracks with BIB contamination
         * @return A Track Collection SoftKiller applied
         */
	std::tuple<edm4hep::TrackerHitPlaneCollection, edm4hep::TrackerHitPlaneCollection> operator()(const edm4hep::TrackerHitPlaneCollection& barrelCollections, const edm4hep::TrackerHitPlaneCollection& endcapCollections) const override;

protected:
	Gaudi::Property<float> m_SideLength{this, "SideLength", 0.9, "Side length of SoftKiller grid."};
	Gaudi::Property<float> m_PhiMax{this, "PhiMax", TMath::Pi(), "Maximum allowed value of phi (absolute value)."};
	Gaudi::Property<float> m_LambdaMax{this, "ThetaMax", TMath::Pi(), "Maximum allowed value of Theta."};
	Gaudi::Property<bool>  m_KeepOverflow{this, "KeepOverflow", true, "Should the algorithm keep or remove all overflow."};
	Gaudi::Property<bool>  m_usePt{this, "UsePt", true, "Should the algorithm use pT or E as the variable."};

	TH3* m_gridMaxes;
	TH1* m_hptCuts;
	
	float m_Bz = 3.57;
};
#endif
