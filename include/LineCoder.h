#pragma once
#include <vector>
#include <string>
#include <memory>

// A signal is stored as TWO voltage samples per bit interval: the level
// during the first half of the bit and the level during the second half.
// Schemes that don't need half-bit resolution (NRZ-L, NRZ-I, AMI, B8ZS,
// HDB3) simply repeat the same value in both halves; Manchester and
// Differential Manchester use the two halves to carry the mid-bit clock
// transition. This single representation lets every decoder work purely
// off the *signal* (voltage levels/transitions), never the original bits.
using SignalLevels = std::vector<double>;

// Abstract base class for every line coding scheme. Concrete schemes
// implement encode()/decode() polymorphically -- there is deliberately no
// central if/else or switch dispatching on "scheme type" anywhere.
class LineCoder {
public:
    virtual ~LineCoder() = default;
    virtual std::string name() const = 0;

    // bits -> physical signal
    virtual SignalLevels encode(const std::vector<int>& bits) = 0;

    // physical signal -> bits. IMPORTANT: implementations only look at the
    // signal's voltage levels/transitions, exactly as a real receiver
    // would -- the original bit vector is never passed in here.
    virtual std::vector<int> decode(const SignalLevels& signal) const = 0;

    static int bitCount(const SignalLevels& signal) { return static_cast<int>(signal.size() / 2); }
};

using LineCoderPtr = std::unique_ptr<LineCoder>;

// ---- NRZ-L: 0 -> low (-1V), 1 -> high (+1V) --------------------------------
class NRZL : public LineCoder {
public:
    std::string name() const override { return "NRZ-L"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
};

// ---- NRZ-I: 1 -> transition at start of bit, 0 -> no transition ----------
class NRZI : public LineCoder {
public:
    explicit NRZI(double initialLevel = -1.0) : initialLevel_(initialLevel) {}
    std::string name() const override { return "NRZ-I"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
private:
    double initialLevel_;
};

// ---- Manchester: 0 -> high-to-low transition mid-bit, 1 -> low-to-high --
class Manchester : public LineCoder {
public:
    std::string name() const override { return "Manchester"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
};

// ---- Differential Manchester: always a mid-bit transition (clock); ------
// ---- additionally a transition at the START of the bit for a 0, none for 1
class DifferentialManchester : public LineCoder {
public:
    explicit DifferentialManchester(double initialLevel = -1.0) : initialLevel_(initialLevel) {}
    std::string name() const override { return "Differential Manchester"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
private:
    double initialLevel_;
};

// ---- AMI: 0 -> 0V, 1 -> alternating +V/-V pulses -------------------------
class AMI : public LineCoder {
public:
    std::string name() const override { return "AMI"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
};

// ---- B8ZS: AMI + substitutes any run of 8 consecutive zeros with ---------
// ---- 000VB0VB (polarity chosen so V breaks alternation) ------------------
class B8ZS : public LineCoder {
public:
    std::string name() const override { return "AMI + B8ZS scrambling"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
};

// ---- HDB3: AMI + substitutes any run of 4 consecutive zeros with ---------
// ---- 000V or B00V, chosen to keep the pulse count between violations even
class HDB3 : public LineCoder {
public:
    std::string name() const override { return "AMI + HDB3 scrambling"; }
    SignalLevels encode(const std::vector<int>& bits) override;
    std::vector<int> decode(const SignalLevels& signal) const override;
};
