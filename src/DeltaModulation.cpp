#include "DeltaModulation.h"

std::vector<int> DeltaModulation::encode(const std::vector<double>& samples) {
    std::vector<int> bits;
    bits.reserve(samples.size());
    double staircase = samples.empty() ? 0.0 : samples.front();

    for (double s : samples) {
        if (s > staircase) {
            bits.push_back(1);
            staircase += stepSize_;
        } else {
            bits.push_back(0);
            staircase -= stepSize_;
        }
    }
    return bits;
}

std::vector<double> DeltaModulation::decode(const std::vector<int>& bits) const {
    std::vector<double> staircase;
    staircase.reserve(bits.size());
    double level = 0.0;
    for (int b : bits) {
        level += (b == 1) ? stepSize_ : -stepSize_;
        staircase.push_back(level);
    }
    return staircase;
}
