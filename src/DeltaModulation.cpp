
#include "DeltaModulation.h"

std::vector<int> DeltaModulation::encode(const std::vector<double>& samples) {
    std::vector<int> encodedBits;
    encodedBits.reserve(samples.size());

    double currentLevel = samples.empty() ? 0.0 : samples[0];

    for (int i = 0; i < samples.size(); i++) {
        int bitValue = 0;

        if (samples[i] > currentLevel) {
            bitValue = 1;
            currentLevel = currentLevel + stepSize_;
        } 
        else {
            currentLevel = currentLevel - stepSize_;
        }

        encodedBits.push_back(bitValue);
    }

    return encodedBits;
}

std::vector<double> DeltaModulation::decode(const std::vector<int>& bits) {
    std::vector<double> decodedSignal;
    decodedSignal.reserve(bits.size());

    double previousLevel = 0.0;

    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == 1) {
            previousLevel += stepSize_;
        } 
        else {
            previousLevel -= stepSize_;
        }

        decodedSignal.push_back(previousLevel);
    }

    return decodedSignal;
}
