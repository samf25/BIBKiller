#include "KillAnalysis.hxx"
#include <GaudiKernel/ITHistSvc.h>


DECLARE_COMPONENT(KillAnalysis)

KillAnalysis::KillAnalysis(const std::string& name, ISvcLocator* svcLoc) : MultiTransformer(name, svcLoc,
    {   KeyValues("UnkilledHits", {"Barrel"}),
        KeyValues("KilledHits", {"Endcap"}),
        KeyValues("Links", {"Links"})
    },
	{   KeyValues("OutputCollection", {"ValueStorage"}) }) {}

// Implement Initializer
StatusCode KillAnalysis::initialize() {
    return StatusCode::SUCCESS;
}

std::tuple<edm4hep::CalorimeterHitCollection> KillAnalysis::operator()(
    const std::vector<const edm4hep::CalorimeterHitCollection*>& unKilledHits,
    const std::vector<const edm4hep::CalorimeterHitCollection*>& killedHits,
    const std::vector<const edm4hep::CaloHitSimCaloHitLinkCollection*>& links) const {

    // Implement the analysis logic here
    edm4hep::CalorimeterHitCollection outputCollection;
    for (int index = 0; index < unKilledHits.size(); ++index) {
        // Map from CalorimeterHit ObjectID to SimCalorimeterHit pointer
        std::unordered_map<uint64_t, std::pair<std::vector<edm4hep::CaloHitContribution>::const_iterator, std::vector<edm4hep::CaloHitContribution>::const_iterator>> hitToSimHitMap;

        auto& linkCollections = links[index];
        for (const auto& link : *linkCollections) {
            uint64_t ID = makeUniqueID(link.getFrom().getObjectID());
            hitToSimHitMap[ID] = std::make_pair(link.getTo().contributions_begin(), link.getTo().contributions_end());
        }
        
        // Process the unKilledHits and killedHits
        auto& unKilledHitCollection = unKilledHits[index];
        auto& killedHitCollection = killedHits[index];

        int BIB = 0;
        int signal = 0;
        int killBIB = 0;
        int killSignal = 0;

        float BIBEnergy = 0.0;
        float signalEnergy = 0.0;
        float killBIBEnergy = 0.0;
        float killSignalEnergy = 0.0;

        // Histogram for unKilledHits
        if (unKilledHitCollection) {
            for (const auto& hit : *unKilledHitCollection) {
                auto itPair = hitToSimHitMap.find(makeUniqueID(hit.getObjectID()));
                int internalSignal = 0;
                int internalBIB = 0;
                if (itPair != hitToSimHitMap.end()) {
                    auto begin = itPair->second.first;
                    auto end = itPair->second.second;
                    for (auto it = begin; it != end; ++it) {
                        if (it->getParticle().isAvailable()) { internalSignal++; }
                        else { internalBIB++; }
                    }
                }
                if (internalSignal > internalBIB) {
                    signal++;
                    signalEnergy += hit.getEnergy();
                } else {
                    BIB++;
                    BIBEnergy += hit.getEnergy();
                }
            }
        }
        BIB -= signal; // BIB hits are those that are not signal

        // Histogram for killedHits
        if (killedHitCollection) {
            for (const auto& hit : *killedHitCollection) {
                auto itPair = hitToSimHitMap.find(makeUniqueID(hit.getObjectID()));
                int internalSignal = 0;
                int internalBIB = 0;
                if (itPair != hitToSimHitMap.end()) {
                    auto begin = itPair->second.first;
                    auto end = itPair->second.second;
                    for (auto it = begin; it != end; ++it) {
                        if (it->getParticle().isAvailable()) { internalSignal++; }
                        else { internalBIB++; }
                    }
                }
                if (internalSignal > internalBIB) {
                    killSignal++;
                    killSignalEnergy += hit.getEnergy();
                } else {
                    killBIB++;
                    killBIBEnergy += hit.getEnergy();
                }
            }
        }
        killBIB -= killSignal; // Killed BIB hits are those that are not signal
        // Encode the four values into a uint64_t: [signal (16 bits)][BIB (16 bits)][killSignal (16 bits)][killBIB (16 bits)]

        // Store the encoded value in the outputCollection's last hit's energy (or as a cellID, or as a parameter, as needed)
        auto hit = outputCollection.create();
        hit.setCellID(signal);
        hit.setPosition(edm4hep::Vector3f(BIBEnergy, signalEnergy, killBIBEnergy)); // Example usage of position to store additional info
        hit.setEnergy(killSignalEnergy); // Using energy to store the killSignal
        hit.setTime(BIB);
        hit.setEnergyError(killBIB); // Using type to store the killBIB
        hit.setType(killSignal); // Using quality to store the killSignal

    }

    return std::make_tuple(std::move(outputCollection));
}