#ifndef KillAnalysis_h
#define KillAnalysis_h 1

// edm4hep
#include <edm4hep/CaloHitSimCaloHitLinkCollection.h>
#include <edm4hep/CalorimeterHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/CaloHitSimCaloHitLink.h>
#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/CalorimeterHit.h>
#include <edm4hep/MCParticle.h>

// k4FWCore
#include <k4FWCore/DataHandle.h>
#include <k4FWCore/Transformer.h>

#include <TH1.h>
#include <TH3.h>
#include <TMath.h>

#include <tuple>


//! \brief Analysis the performance of the BIBKiller
/**
 * Connect Hits back to MCParticles and analyze if BIBKiller
 * removed the BIB hits.
 * 
 * @author Samuel Ferraro
 * @version $Id$
 */

struct KillAnalysis final : k4FWCore::MultiTransformer<std::tuple<edm4hep::CalorimeterHitCollection>(
    const std::vector<const edm4hep::CalorimeterHitCollection*>&,
    const std::vector<const edm4hep::CalorimeterHitCollection*>&,
    const std::vector<const edm4hep::CaloHitSimCaloHitLinkCollection*>&
)> {
public:
    /**
     * @brief Constructer for the KillAnalysis
     * @param name unique string identifier for this instance
     * @param svcLoc a Service Locator passed by the Gaudi AlgManager 
     */
    KillAnalysis(const std::string& name, ISvcLocator* pSvcLocator);

    /**
     * @brief Initialize the KillAnalysis
     */
    StatusCode initialize();

    /**
     * @brief The KillAnalysis operation. The workhorse of this Transformer.
     * @param unKilledHits The collection of calorimeter hits before BIBKiller
     * @param killedHits The collection of calorimeter hits after BIBKiller
     */ 
    std::tuple<edm4hep::CalorimeterHitCollection> operator()(
        const std::vector<const edm4hep::CalorimeterHitCollection*>& unKilledHits,
        const std::vector<const edm4hep::CalorimeterHitCollection*>& killedHits,
        const std::vector<const edm4hep::CaloHitSimCaloHitLinkCollection*>& links) const override;

private:
    uint64_t makeUniqueID(podio::ObjectID id) const{
        return (static_cast<uint64_t>(id.collectionID) << 32) | (static_cast<uint32_t>(id.index));
    }
};



#endif