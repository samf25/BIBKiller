#ifndef BIBKiller_h
#define BIBKiller_h 1

// edm4hep
#include <edm4hep/TrackCollection.h>
#include <edm4hep/Track.h>

// Gaudi
#include <GaudiAlg/GaudiAlgorithm.h>
#include <GaudiAlg/Transformer.h>
#include <k4FWCore/BaseClass.h>

// k4FWCore
#include <k4FWCore/DataHandle.h>

#include <TH1.h>
#include <TMath.h>

#include <tuple>

#include "SoftBox.hxx"

//! \brief Apply the SoftDrop Algorithm
/**
 * Remove tracks until half the grids
 * are empty.
 *
 * @author Samuel Ferraro
 * @version $Id$
 */
struct BIBKiller final : Gaudi::Functional::Transformer <edm4hep::TrackCollection(
	const edm4hep::TrackCollection&)> {
public:
	/**
         * @brief Constructor for BIBKiller
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	BIBKiller(const std::string& name, ISvcLocator* svcLoc);

	/**
 	 * @brief Register and create all histograms (and Histogram Service)
 	 */
	StatusCode initialize();
		
	/**
         * @brief BIBKiller operation. The workhorse of this Transformer.
         * @param trackCollection A collection of reconstructed tracks with BIB contamination
         * @return A Track Collection SoftKiller applied
         */
	edm4hep::TrackCollection operator()(const edm4hep::TrackCollection& trackCollection) const override;

protected:
	Gaudi::Property<int> m_nPhiRows{this, "nPhiRows", 10, "Number of grid rows in phi."};
	Gaudi::Property<int> m_nLambdaCols{this, "nLambdaCols", 10, "Number of grid columns in lambda."};
	Gaudi::Property<float> m_PhiMax{this, "PhiMax", 2*TMath::Pi(), "Maximum allowed value of phi (absolute value)."};
	Gaudi::Property<float> m_LambdaMax{this, "LambdaMax", 1.4, "Maximum allowed value of Lambda (absolute value)."};
	Gaudi::Property<bool> m_KeepOverflow{this, "KeepOverflow", true, "Should the algorithm keep or remove all overflow."};



	TH1* m_hptCuts;
	
	float m_Bz = 3.57;
};
#endif
