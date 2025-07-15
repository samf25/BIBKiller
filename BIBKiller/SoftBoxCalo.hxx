#ifndef SoftBoxCalo_h
#define SoftBoxCalo_h 1

#include <edm4hep/CalorimeterHit.h>
#include <vector>
#include <utility>
#include <tuple>

struct CaloInfo {
    int index;
	uint32_t col;
    float val;
};

class SoftBoxCalo {
public:
	SoftBoxCalo();  // Constructor declaration
	std::tuple<float, float> addCalo(uint32_t col, int index, float energy, float theta, bool usePt);  // Method to add a calo hit
	float getMax() const;  // Method to get the maximum value
	std::vector<CaloInfo> getCalos() const; // Method to retrieve all calo hit and Vals
private:
	std::vector<CaloInfo> caloValPairs;  // Vector to hold pointers to calo hit
	float maxVal;  // Variable to track the maximum value
};

#endif // SOFTBOXCALO_H

