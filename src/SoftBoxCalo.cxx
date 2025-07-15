#include <TMath.h>

#include "SoftBoxCalo.hxx"
#include <iostream>

// Constructor initializes maxPt to 0
SoftBoxCalo::SoftBoxCalo() { maxVal = 0; }

// Method to add a Track pointer and update maxPt if needed
std::tuple<float, float> SoftBoxCalo::addCalo(uint32_t col, int index, float energy, float theta, bool usePt) {
	float val = 0;
	if (usePt) { val = std::abs(energy * TMath::Cos(theta)); }
	else { val = energy; }

	CaloInfo info(index, col, val);
	caloValPairs.emplace_back(info);
	if (val > maxVal) { maxVal = val; }

	return std::make_tuple(val, maxVal);
}

// Method to retrieve the maximum value
float SoftBoxCalo::getMax() const { return maxVal; }

// Method to retieve Calo and their values
std::vector<CaloInfo> SoftBoxCalo::getCalos() const { return caloValPairs; }
