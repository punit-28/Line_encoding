#include "PCM.h"
#include <algorithm>
#include <cmath>

PCM::PCM(int quantizationBits, double minVoltage, double maxVoltage)
    : quantizationBits_(quantizationBits), minVoltage_(minVoltage), maxVoltage_(maxVoltage) {
    stepSize_ = (maxVoltage_ - minVoltage_) / static_cast<double>(levelCount());
}

std::vector<int> PCM::encode(const std::vector<double>& samples, std::vector<int>* outLevels) {
    std::vector<int> bits;
    bits.reserve(samples.size() * quantizationBits_);
    int maxLevel = levelCount() - 1;

    for (double s : samples) {
        double clamped = std::clamp(s, minVoltage_, maxVoltage_ - 1e-9);
        int level = static_cast<int>(std::floor((clamped - minVoltage_) / stepSize_));
        level = std::clamp(level, 0, maxLevel);
        if (outLevels) outLevels->push_back(level);

        for (int b = quantizationBits_ - 1; b >= 0; --b) {
            bits.push_back((level >> b) & 1);
        }
    }
    return bits;
}

std::vector<double> PCM::decode(const std::vector<int>& bits) const {
    std::vector<double> staircase;
    int samples = static_cast<int>(bits.size()) / quantizationBits_;
    staircase.reserve(samples);

    for (int s = 0; s < samples; ++s) {
        int level = 0;
        for (int b = 0; b < quantizationBits_; ++b) {
            level = (level << 1) | bits[s * quantizationBits_ + b];
        }
        // we use the midpoint of the quantization interval to reconstruct the analog value. This is calculated by taking the minimum voltage and adding the product of the level (plus 0.5 for midpoint) and the step size. This ensures that we are accurately representing the quantized value in the analog domain.
        // Reconstruct as the midpoint of the quantization interval. here this we want to check also double over flow beacause the range is 10^15;
        double voltage = minVoltage_ + (level + 0.5) * stepSize_;
        staircase.push_back(voltage);
    }
    return staircase;
}
