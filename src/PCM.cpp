
#include "PCM.h"
#include <algorithm>
#include <cmath>

PCM::PCM(int quantizationBits, double minVoltage, double maxVoltage)
    : quantizationBits_(quantizationBits),
      minVoltage_(minVoltage),
      maxVoltage_(maxVoltage) {

    stepSize_ = (maxVoltage_ - minVoltage_) / levelCount();
}

std::vector<int> PCM::encode(
    const std::vector<double>& samples, std::vector<int>* outLevels) {

    std::vector<int> encodedData;
    encodedData.reserve(samples.size() * quantizationBits_);

    int highestLevel = levelCount() - 1;

    for (int i = 0; i < samples.size(); i++) {

        double voltage = std::clamp(
            samples[i], minVoltage_, maxVoltage_ - 1e-9
        );

        int quantizedLevel = static_cast<int>(
            std::floor((voltage - minVoltage_) / stepSize_)
        );

        quantizedLevel = std::clamp(quantizedLevel, 0, highestLevel);

        if (outLevels != nullptr) {
            outLevels->push_back(quantizedLevel);
        }

        for (int bit = quantizationBits_ - 1; bit >= 0; bit--) {
            int currentBit = (quantizedLevel >> bit) & 1;
            encodedData.push_back(currentBit);
        }
    }

    return encodedData;
}

std::vector<double> PCM::decode(const std::vector<int>& bits) const {

    std::vector<double> reconstructed;
    int totalSamples = static_cast<int>(bits.size()) / quantizationBits_;

    reconstructed.reserve(totalSamples);

    for (int i = 0; i < totalSamples; i++) {

        int decodedLevel = 0;

        for (int j = 0; j < quantizationBits_; j++) {
            int bitPosition = i * quantizationBits_ + j;

            decodedLevel = (decodedLevel << 1) | bits[bitPosition];
        }

        double reconstructedValue =
            minVoltage_ + (decodedLevel + 0.5) * stepSize_;

        reconstructed.push_back(reconstructedValue);
    }

    return reconstructed;
}
