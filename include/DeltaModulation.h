#pragma once
#include <vector>

// Linear Delta Modulation: 1 bit per sample, encoding whether the signal
// rose or fell relative to a running staircase approximation.
class DeltaModulation {
public:
    explicit DeltaModulation(double stepSize) : stepSize_(stepSize) {}

    // 1 -> signal was above the staircase (staircase += step)
    // 0 -> signal was at/below the staircase (staircase -= step)
    std::vector<int> encode(const std::vector<double>& samples);

    // Rebuilds the decoder's staircase purely from the bitstream.
    std::vector<double> decode(const std::vector<int>& bits) const;

private:
    double stepSize_;
};
