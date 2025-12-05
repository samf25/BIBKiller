#ifndef LayerSelector_h
#define LayerSelector_h 1

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

//! \brief Select Specific Calorimeter Layer
/**
 * Remove tracks until half the grids
 * are empty.
 *
 * @author Samuel Ferraro
 * @version $Id$
 */
struct LayerSelector final : k4FWCore::MultiTransformer <std::tuple<edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection>(const std::vector<const edm4hep::CalorimeterHitCollection*>&, const std::vector<const edm4hep::CalorimeterHitCollection*>&)> {
public:
	/**
         * @brief Constructor for LayerSelector
         * @param name unique string identifier for this instance
         * @param svcLoc a Service Locator passed by the Gaudi AlgManager
         */
	LayerSelector(const std::string& name, ISvcLocator* svcLoc);

	/**
 	 * @brief Register and create all histograms (and Histogram Service)
 	 */
	StatusCode initialize();
		
	/**
         * @brief BIBKiller operation. The workhorse of this Transformer.
         */
	std::tuple<edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection> operator()(const std::vector<const edm4hep::CalorimeterHitCollection*>& barrelCollections, const std::vector<const edm4hep::CalorimeterHitCollection*>& endcapCollections) const override;

protected:
	Gaudi::Property<int> m_minLayer{this, "MinLayer", 3, "Minimum layer of Calorimeter."};
	Gaudi::Property<int> m_maxLayer{this, "MaxLayer", 5, "Maximum layer of Calorimeter."};
};
#endif
