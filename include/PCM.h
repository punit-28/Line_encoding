#pragma once
#include <vector>
#include <cstdint>

// Pulse Code Modulation: sample -> quantize -> binary-encode -> (later) decode.
class PCM {
public:
    explicit PCM(int quantizationBits, double minVoltage = -1.0, double maxVoltage = 1.0);

    // Quantizes each analog sample to the nearest of 2^bits levels and
    // returns the concatenated binary bitstream (quantizationBits_ bits
    // per sample, MSB first). `outLevels` (optional) receives the integer
    // quantization level chosen for each sample, for reporting/plotting.
    std::vector<int> encode(const std::vector<double>& samples, std::vector<int>* outLevels = nullptr);

    // Reconstructs the quantized analog signal (the decoder's staircase)
    // purely from the bitstream -- no access to the original samples.
    std::vector<double> decode(const std::vector<int>& bits) const;

    int quantizationBits() const { return quantizationBits_; }
    int levelCount() const { return 1 << quantizationBits_; }

private:
    int quantizationBits_;
    double minVoltage_;
    double maxVoltage_;
    double stepSize_;
};
