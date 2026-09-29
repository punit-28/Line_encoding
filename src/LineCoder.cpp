#include "LineCoder.h"
#include <cmath>

namespace {
    inline double sgn(double v) { return v > 0 ? 1.0 : (v < 0 ? -1.0 : 0.0); }
}

// ============================== NRZ-L ======================================
SignalLevels NRZL::encode(const std::vector<int>& bits) {
    SignalLevels sig(bits.size() * 2);
    for (size_t i = 0; i < bits.size(); ++i) {
        double level = bits[i] ? 1.0 : -1.0;
        sig[2 * i] = level;
        sig[2 * i + 1] = level;
    }
    return sig;
}

std::vector<int> NRZL::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<int> bits(n);
    for (int i = 0; i < n; ++i) {
        double level = (signal[2 * i] + signal[2 * i + 1]) / 2.0;
        bits[i] = level > 0 ? 1 : 0;
    }
    return bits;
}

// ============================== NRZ-I ======================================
SignalLevels NRZI::encode(const std::vector<int>& bits) {
    SignalLevels sig(bits.size() * 2);
    double level = initialLevel_;
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == 1) level = -level; // transition represents a 1
        sig[2 * i] = level;
        sig[2 * i + 1] = level;
    }
    return sig;
}

std::vector<int> NRZI::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<int> bits(n);
    double prevLevel = initialLevel_;
    for (int i = 0; i < n; ++i) {
        double level = signal[2 * i];
        bits[i] = (sgn(level) != sgn(prevLevel)) ? 1 : 0;
        prevLevel = level;
    }
    return bits;
}

// ============================== Manchester =================================
// Convention: 0 -> first half HIGH, second half LOW (high-to-low transition)
//             1 -> first half LOW,  second half HIGH (low-to-high transition)
SignalLevels Manchester::encode(const std::vector<int>& bits) {
    SignalLevels sig(bits.size() * 2);
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] == 0) { sig[2 * i] = 1.0; sig[2 * i + 1] = -1.0; }
        else              { sig[2 * i] = -1.0; sig[2 * i + 1] = 1.0; }
    }
    return sig;
}

std::vector<int> Manchester::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<int> bits(n);
    for (int i = 0; i < n; ++i) {
        double firstHalf = signal[2 * i];
        double secondHalf = signal[2 * i + 1];
        // low-to-high transition => 1, high-to-low => 0
        bits[i] = (secondHalf > firstHalf) ? 1 : 0;
    }
    return bits;
}

// ===================== Differential Manchester ==============================
// Always a mid-bit transition (clock). A transition at the START of the bit
// (relative to the previous bit's ending level) encodes a 0; no start-of-bit
// transition encodes a 1.
SignalLevels DifferentialManchester::encode(const std::vector<int>& bits) {
    SignalLevels sig(bits.size() * 2);
    double prevEndLevel = initialLevel_;
    for (size_t i = 0; i < bits.size(); ++i) {
        double firstHalf = (bits[i] == 0) ? -prevEndLevel : prevEndLevel;
        double secondHalf = -firstHalf; // mandatory mid-bit clock transition
        sig[2 * i] = firstHalf;
        sig[2 * i + 1] = secondHalf;
        prevEndLevel = secondHalf;
    }
    return sig;
}

std::vector<int> DifferentialManchester::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<int> bits(n);
    double prevEndLevel = initialLevel_;
    for (int i = 0; i < n; ++i) {
        double firstHalf = signal[2 * i];
        // transition at bit-start (differs from previous bit's ending level) => 0
        bits[i] = (sgn(firstHalf) != sgn(prevEndLevel)) ? 0 : 1;
        prevEndLevel = signal[2 * i + 1];
    }
    return bits;
}

// ============================== AMI ========================================
SignalLevels AMI::encode(const std::vector<int>& bits) {
    SignalLevels sig(bits.size() * 2);
    double lastPolarity = -1.0; // so the first 1-bit becomes +1
    for (size_t i = 0; i < bits.size(); ++i) {
        double level = 0.0;
        if (bits[i] == 1) {
            level = -lastPolarity;
            lastPolarity = level;
        }
        sig[2 * i] = level;
        sig[2 * i + 1] = level;
    }
    return sig;
}

std::vector<int> AMI::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<int> bits(n);
    for (int i = 0; i < n; ++i) {
        bits[i] = (sgn(signal[2 * i]) != 0) ? 1 : 0;
    }
    return bits;
}

// ============================== B8ZS =======================================
// Encodes like AMI, but whenever 8 CONSECUTIVE zero bits are seen, that
// 8-bit block is replaced with the pulse pattern  0 0 0 V B 0 V B
// where V has the SAME polarity as the pulse immediately preceding the
// block (a deliberate bipolar-rule violation) and B follows the normal
// alternating rule.
SignalLevels B8ZS::encode(const std::vector<int>& bits) {
    int n = static_cast<int>(bits.size());
    std::vector<double> pulses(n, 0.0);
    double lastPolarity = -1.0;
    int zeroRun = 0;

    for (int i = 0; i < n; ++i) {
        if (bits[i] == 1) {
            pulses[i] = -lastPolarity;
            lastPolarity = pulses[i];
            zeroRun = 0;
        } else {
            pulses[i] = 0.0;
            ++zeroRun;
            if (zeroRun == 8) {
                double V = lastPolarity;   // violation: repeats previous polarity
                double B = -lastPolarity;  // normal alternation
                int s = i - 7;
                pulses[s + 0] = 0; pulses[s + 1] = 0; pulses[s + 2] = 0;
                pulses[s + 3] = V; pulses[s + 4] = B; pulses[s + 5] = 0;
                pulses[s + 6] = V; pulses[s + 7] = B;
                lastPolarity = B;
                zeroRun = 0;
            }
        }
    }

    SignalLevels sig(n * 2);
    for (int i = 0; i < n; ++i) { sig[2 * i] = pulses[i]; sig[2 * i + 1] = pulses[i]; }
    return sig;
}

std::vector<int> B8ZS::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<double> pulses(n);
    for (int i = 0; i < n; ++i) pulses[i] = signal[2 * i];

    std::vector<int> bits(n, 0);
    // Must match the encoder's assumed initial polarity reference (-1.0),
    // exactly like NRZ-I / Differential Manchester assume a known initial
    // level -- otherwise a violation at the very start of the stream
    // (nothing came before it) would be indistinguishable from a normal
    // pulse.
    double lastPolarity = -1.0;
    int i = 0;
    while (i < n) {
        if (sgn(pulses[i]) == 0.0) { bits[i] = 0; ++i; continue; }

        if (sgn(pulses[i]) == sgn(lastPolarity)) {
            // Bipolar-rule violation detected at index i -> this is the
            // first "V" of a B8ZS block. The block spans [i-3, i+4].
            int s = i - 3, e = i + 4;
            if (s >= 0 && e < n) {
                for (int k = s; k <= e; ++k) bits[k] = 0; // whole block was a run of zeros
                lastPolarity = sgn(pulses[e]); // trailing B's polarity
                i = e + 1;
                continue;
            }
        }
        // ordinary pulse (real 1-bit)
        bits[i] = 1;
        lastPolarity = sgn(pulses[i]);
        ++i;
    }
    return bits;
}

// ============================== HDB3 ========================================
// Encodes like AMI, but whenever 4 CONSECUTIVE zero bits are seen, that
// 4-bit block is replaced with either "000V" (if an odd number of 1-bits
// have occurred since the previous substitution) or "B00V" (if an even
// number, including zero, have occurred). This keeps the number of
// non-zero pulses between violations even, preserving DC balance.
SignalLevels HDB3::encode(const std::vector<int>& bits) {
    int n = static_cast<int>(bits.size());
    std::vector<double> pulses(n, 0.0);
    double lastPolarity = -1.0;
    int zeroRun = 0;
    int onesSinceSub = 0;

    for (int i = 0; i < n; ++i) {
        if (bits[i] == 1) {
            pulses[i] = -lastPolarity;
            lastPolarity = pulses[i];
            ++onesSinceSub;
            zeroRun = 0;
        } else {
            pulses[i] = 0.0;
            ++zeroRun;
            if (zeroRun == 4) {
                int s = i - 3;
                if (onesSinceSub % 2 == 1) {
                    // "000V" : violation repeats the last pulse's polarity
                    double V = lastPolarity;
                    pulses[s + 0] = 0; pulses[s + 1] = 0; pulses[s + 2] = 0; pulses[s + 3] = V;
                    lastPolarity = V;
                } else {
                    // "B00V" : B alternates normally, V repeats B's polarity
                    double B = -lastPolarity;
                    double V = B;
                    pulses[s + 0] = B; pulses[s + 1] = 0; pulses[s + 2] = 0; pulses[s + 3] = V;
                    lastPolarity = V;
                }
                onesSinceSub = 0;
                zeroRun = 0;
            }
        }
    }

    SignalLevels sig(n * 2);
    for (int i = 0; i < n; ++i) { sig[2 * i] = pulses[i]; sig[2 * i + 1] = pulses[i]; }
    return sig;
}

std::vector<int> HDB3::decode(const SignalLevels& signal) const {
    int n = bitCount(signal);
    std::vector<double> pulses(n);
    for (int i = 0; i < n; ++i) pulses[i] = signal[2 * i];

    std::vector<int> bits(n, 0);
    double lastPolarity = -1.0; // same initial reference the encoder assumes
    int i = 0;
    while (i < n) {
        if (sgn(pulses[i]) == 0.0) { bits[i] = 0; ++i; continue; }

        if (sgn(pulses[i]) == sgn(lastPolarity)) {
            // Violation "V" found at i -> 4-symbol block is [i-3, i]
            int s = i - 3;
            if (s >= 0) {
                for (int k = s; k <= i; ++k) bits[k] = 0; // whole block was a run of zeros
                lastPolarity = sgn(pulses[i]);
                i = i + 1;
                continue;
            }
        }
        bits[i] = 1;
        lastPolarity = sgn(pulses[i]);
        ++i;
    }
    return bits;
}
