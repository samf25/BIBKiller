#ifndef BIBKillerCluster_h
#define BIBKillerCluster_h 1

// edm4hep
#include <edm4hep/ClusterCollection.h>
#include <edm4hep/Cluster.h>

// k4FWCore
#include <k4FWCore/DataHandle.h>
#include <k4FWCore/Transformer.h>

#include <TH1.h>
#include <TMath.h>

#include <tuple>

#include "SoftBoxCluster.hxx"

//! \brief Apply the SoftDrop Algorithm
/**
 * Remove tracks until half the grids
 * are empty.
 *
 * @author Samuel Ferraro
 * @version $Id$
 */
struct BIBKillerCluster final : k4FWCore::Transformer <edm4hep::ClusterCollection(
	const edm4hep::ClusterCollection&)> {
public:
	/**
         * @brief Constructor for BIBKiller
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	BIBKillerCluster(const std::string& name, ISvcLocator* svcLoc);

	/**
 	 * @brief Register and create all histograms (and Histogram Service)
 	 */
	StatusCode initialize();
		
	/**
         * @brief BIBKiller operation. The workhorse of this Transformer.
         * @param trackCollection A collection of reconstructed tracks with BIB contamination
         * @return A Track Collection SoftKiller applied
         */
	edm4hep::ClusterCollection operator()(const edm4hep::ClusterCollection& clusterCollection) const override;

protected:
	Gaudi::Property<float> m_SideLength{this, "SideLength", 0.9, "Side length of SoftKiller grid."};
	Gaudi::Property<float> m_PhiMax{this, "PhiMax", TMath::Pi(), "Maximum allowed value of phi (absolute value)."};
	Gaudi::Property<float> m_LambdaMax{this, "LambdaMax", TMath::Pi()/4, "Maximum allowed value of Lambda (absolute value)."};
	Gaudi::Property<bool>  m_KeepOverflow{this, "KeepOverflow", true, "Should the algorithm keep or remove all overflow."};
	Gaudi::Property<bool>  m_usePt{this, "UsePt", true, "Should the algorithm use pT or E as the variable."}

	TH1* m_hptCuts;
	
	float m_Bz = 3.57;
};
#endif
