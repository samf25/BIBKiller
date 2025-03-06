#include <edm4hep/TrackState.h>
#include <edm4hep/MutableTrack.h>

#include <GaudiKernel/ITHistSvc.h>

#include <utility>
#include <tuple>

#include "BIBKiller.hxx"
#include "SoftBox.hxx"

DECLARE_COMPONENT(BIBKiller)

BIBKiller::BIBKiller(const std::string& name, ISvcLocator* svcLoc) : Transformer(name, svcLoc,
	 KeyValues("InputTrackCollectionName", {"Tracks"}),
	 KeyValues("OutputTrackCollectionName", {"BIBKilledTracks"})) {}

// Implement Initializer
StatusCode BIBKiller::initialize() {
        // Get Histogram and Data Services
        ITHistSvc* histSvc{nullptr};
        StatusCode sc1 = service("THistSvc", histSvc);
        if ( sc1.isFailure() ) {
                error() << "Could not locate HistSvc" << endmsg;
                return StatusCode::FAILURE; }

        // Make Histogram
        m_hptCuts = new TH1F("SoftKiller pT Cuts", "pTCut;Events", 100, 0, 100);
        (void)histSvc->regHist("/histos/all/pTCuts", m_hptCuts);

        int nLamb = 2*std::floor(m_LambdaMax.value()/m_SideLength.value())+2;
        int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
	m_gridMaxes = new TH3F("SoftBox pT Maxes", "Lambda;Phi",
								nLamb, 0, nLamb, nPhi, 0, nPhi,
//								2*std::floor(m_LambdaMax.value()/m_SideLength.value())+2, 0, m_LambdaMax.value(),
//								2*std::floor(m_PhiMax.value()/m_SideLength.value())+2, 0, m_PhiMax.value(),
								100, 0, 100);
	(void)histSvc->regHist("/histos/all/gridMaxes", m_gridMaxes);

        return StatusCode::SUCCESS;
}

edm4hep::TrackCollection BIBKiller::operator()(
		const edm4hep::TrackCollection& trackCollection) const{
	MsgStream log(msgSvc(), name());

	// Make output collection
	edm4hep::TrackCollection outputTracks;
	outputTracks.setSubsetCollection();
	// Fill SoftKiller Grid
	int nLamb = 2*std::floor(m_LambdaMax.value()/m_SideLength.value())+2;
	int nPhi = 2*std::floor(m_PhiMax.value()/m_SideLength.value())+2;
	SoftBox grid[nPhi * nLamb];
	for (const auto& track : trackCollection) {
		// Calculate Location
		const edm4hep::TrackState& state = track.getTrackStates(edm4hep::TrackState::AtIP);
		float phi = state.phi;
		float lambda = std::atan(state.tanLambda);
		log << MSG::DEBUG << "\nPhi: " << phi << "\nLambda: " << lambda << endmsg;
		int index = nLamb*std::floor(phi/m_SideLength.value()+nPhi/2)+std::floor(lambda/m_SideLength.value()+nLamb/2);
		(void)grid[index].addTrack(track, m_Bz);
	}	

	// Find median pT of the SoftBoxes
	float ptCut = 0;
	std::vector<float> ptValues;
        // Extract all pT values from SoftBoxes into a vector
        for (int i = 0; i < nPhi * nLamb; ++i) {
                ptValues.push_back(grid[i].getMaxPt());
        }
        // Sort the pT values
        std::sort(ptValues.begin(), ptValues.end());

        // Get the median
        int n = ptValues.size();
        if (n % 2 == 0) {
                // If even, average the two middle values
                ptCut = (ptValues[n/2 - 1] + ptValues[n/2]) / 2.0f;
        } else {
                // If odd, the middle value
                ptCut = ptValues[n/2];
        }	
	
	m_hptCuts->Fill(ptCut);

	// Filter out all tracks below the pT cut
	int count = 0;
	for (int j = 0; j < nPhi * nLamb; j++) {
		log << MSG::DEBUG << "Number of Tracks in box " << count << ": " << grid[j].getTracks().size() << "." << endmsg;
		count++;
		for (std::pair<const edm4hep::Track, float> pair : grid[j].getTracks()) {
			if (pair.second > ptCut) {
				outputTracks.push_back(pair.first);
			}
		}
		m_gridMaxes->Fill(
			j % nLamb,
			std::floor((j-(j%nLamb)) / nLamb),
			(grid[j].getTracks().size() > 0) ? grid[j].getMaxPt() : 0
		);
	}

	return outputTracks;
}

