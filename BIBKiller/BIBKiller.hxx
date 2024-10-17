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
struct BIBKiller final : Gaudi::Functional::Transformer <edm4hep::TrackCollection
(
	const edm4hep::TrackCollection&)> {
public:
	/**
         * @brief Constructor for BIBKiller
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	BIBKiller(const std::string& name, ISvcLocator* svcLoc);
	
	/**
         * @brief BIBKiller operation. The workhorse of this Transformer.
         * @param trackCollection A collection of reconstructed tracks with BIB contamination
         * @return A Track Collection SoftKiller applied
         */
	edm4hep::TrackCollection operator()(const edm4hep::TrackCollection& trackCollection) const override;

protected:
	Gaudi::Property<int> m_nPhiRows{this, "nPhiRows", 10, "Number of grid rows in phi."};
	Gaudi::Property<int> m_nThetaCols{this, "nThetaCols", 10, "Number of grid columns in theta."};
	
	float m_Bz = 3.57;
};
#endif
