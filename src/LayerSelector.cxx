#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>
#include <TMath.h>
#include "DDSegmentation/BitFieldCoder.h"
#include "LayerSelector.hxx"

DECLARE_COMPONENT(LayerSelector)

LayerSelector::LayerSelector(const std::string& name, ISvcLocator* svcLoc) : MultiTransformer(name, svcLoc,
	 { KeyValues("InputBarrelCollections", {"Barrel"}),
           KeyValues("InputEndcapCollections", {"Endcap"}) },
	 { KeyValues("OutputBarrelCollections", {"LayerSelectedBarrel"}),
           KeyValues("OutputEndcapCollections", {"LayerSelectedEndcap"}) }) {}

// Implement Initializer
StatusCode LayerSelector::initialize() {
        return StatusCode::SUCCESS;
}

std::tuple<edm4hep::CalorimeterHitCollection, edm4hep::CalorimeterHitCollection> LayerSelector::operator()(
        const std::vector<const edm4hep::CalorimeterHitCollection*>& barrelCollections,
        const std::vector<const edm4hep::CalorimeterHitCollection*>& endcapCollections) const {
        // Make output collection
	edm4hep::CalorimeterHitCollection outputBarrel;
        edm4hep::CalorimeterHitCollection outputEndcap;
	outputBarrel.setSubsetCollection();
        outputEndcap.setSubsetCollection();
        debug() << "Selecting layers inside range [" << m_minLayer.value() << ", "
                << m_maxLayer.value() << "]" << endmsg;

        dd4hep::DDSegmentation::BitFieldCoder bitFieldCoder("system:5,side:-2,module:8,stave:4,layer:9,submodule:4,x:32:-16,y:-16");

        for (auto& collection : barrelCollections) {
                for (const auto& hit : *collection) {
                        if (bitFieldCoder.get(hit.getCellID(), "layer") >= m_minLayer.value() &&
                            bitFieldCoder.get(hit.getCellID(), "layer") < m_maxLayer.value()) {
                                outputBarrel.push_back(hit);
                        }
                }
        }
        for (auto& collection : endcapCollections) {
                for (const auto& hit : *collection) {
                        if (bitFieldCoder.get(hit.getCellID(), "layer") >= m_minLayer.value() &&
                            bitFieldCoder.get(hit.getCellID(), "layer") < m_maxLayer.value()) {
                                outputEndcap.push_back(hit);
                        }
                }
        }
        debug() << "Selected " << outputBarrel.size() << " barrel hits and "
                << outputEndcap.size() << " endcap hits." << endmsg;

	return std::make_tuple(std::move(outputBarrel), std::move(outputEndcap));
}


