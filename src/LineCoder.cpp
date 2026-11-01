
#include "LineCoder.h"
#include <cmath>

namespace {
    inline double signValue(double num) {
        if (num > 0) return 1.0;
        if (num < 0) return -1.0;
        return 0.0;
    }
}

// NRZ-L Encoding
SignalLevels NRZL::encode(const std::vector<int>& bits) {
    SignalLevels output(bits.size() * 2);

    for (int i = 0; i < bits.size(); i++) {
        double value = (bits[i] == 1) ? 1.0 : -1.0;
        output[i * 2] = value;
        output[i * 2 + 1] = value;
    }

    return output;
}

std::vector<int> NRZL::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<int> result(count);

    for (int i = 0; i < count; i++) {
        double avg = (signal[2 * i] + signal[2 * i + 1]) / 2.0;
        result[i] = (avg > 0) ? 1 : 0;
    }

    return result;
}

// NRZ-I Encoding
SignalLevels NRZI::encode(const std::vector<int>& bits) {
    SignalLevels output(bits.size() * 2);
    double current = initialLevel_;

    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == 1) {
            current *= -1;
        }

        output[2 * i] = current;
        output[2 * i + 1] = current;
    }

    return output;
}

std::vector<int> NRZI::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<int> result(count);
    double previous = initialLevel_;

    for (int i = 0; i < count; i++) {
        double present = signal[2 * i];

        result[i] = (signValue(present) != signValue(previous)) ? 1 : 0;
        previous = present;
    }

    return result;
}

// Manchester Encoding
SignalLevels Manchester::encode(const std::vector<int>& bits) {
    SignalLevels output(bits.size() * 2);

    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == 0) {
            output[2 * i] = 1.0;
            output[2 * i + 1] = -1.0;
        } else {
            output[2 * i] = -1.0;
            output[2 * i + 1] = 1.0;
        }
    }

    return output;
}

std::vector<int> Manchester::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<int> result(count);

    for (int i = 0; i < count; i++) {
        double first = signal[2 * i];
        double second = signal[2 * i + 1];

        result[i] = (second > first) ? 1 : 0;
    }

    return result;
}

// Differential Manchester Encoding
SignalLevels DifferentialManchester::encode(const std::vector<int>& bits) {
    SignalLevels output(bits.size() * 2);
    double previous = initialLevel_;

    for (int i = 0; i < bits.size(); i++) {
        double first = (bits[i] == 0) ? -previous : previous;
        double second = -first;

        output[2 * i] = first;
        output[2 * i + 1] = second;

        previous = second;
    }

    return output;
}

std::vector<int> DifferentialManchester::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<int> result(count);
    double previous = initialLevel_;

    for (int i = 0; i < count; i++) {
        double first = signal[2 * i];

        result[i] = (signValue(first) != signValue(previous)) ? 0 : 1;
        previous = signal[2 * i + 1];
    }

    return result;
}

// AMI Encoding
SignalLevels AMI::encode(const std::vector<int>& bits) {
    SignalLevels output(bits.size() * 2);
    double polarity = -1.0;

    for (int i = 0; i < bits.size(); i++) {
        double value = 0.0;

        if (bits[i] == 1) {
            value = -polarity;
            polarity = value;
        }

        output[2 * i] = value;
        output[2 * i + 1] = value;
    }

    return output;
}

std::vector<int> AMI::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<int> result(count);

    for (int i = 0; i < count; i++) {
        result[i] = (signValue(signal[2 * i]) != 0) ? 1 : 0;
    }

    return result;
}

// B8ZS Encoding
SignalLevels B8ZS::encode(const std::vector<int>& bits) {
    int count = static_cast<int>(bits.size());
    std::vector<double> pulse(count, 0.0);

    double polarity = -1.0;
    int zeros = 0;

    for (int i = 0; i < count; i++) {
        if (bits[i] == 1) {
            pulse[i] = -polarity;
            polarity = pulse[i];
            zeros = 0;
        } else {
            zeros++;

            if (zeros == 8) {
                double violation = polarity;
                double normal = -polarity;
                int start = i - 7;

                pulse[start + 3] = violation;
                pulse[start + 4] = normal;
                pulse[start + 6] = violation;
                pulse[start + 7] = normal;

                polarity = normal;
                zeros = 0;
            }
        }
    }

    SignalLevels output(count * 2);

    for (int i = 0; i < count; i++) {
        output[2 * i] = pulse[i];
        output[2 * i + 1] = pulse[i];
    }

    return output;
}

std::vector<int> B8ZS::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<double> pulse(count);

    for (int i = 0; i < count; i++) {
        pulse[i] = signal[2 * i];
    }

    std::vector<int> result(count, 0);
    double polarity = -1.0;
    int index = 0;

    while (index < count) {
        if (signValue(pulse[index]) == 0) {
            index++;
            continue;
        }

        if (signValue(pulse[index]) == signValue(polarity)) {
            int start = index - 3;
            int end = index + 4;

            if (start >= 0 && end < count) {
                for (int j = start; j <= end; j++) {
                    result[j] = 0;
                }

                polarity = signValue(pulse[end]);
                index = end + 1;
                continue;
            }
        }

        result[index] = 1;
        polarity = signValue(pulse[index]);
        index++;
    }

    return result;
}

// HDB3 Encoding
SignalLevels HDB3::encode(const std::vector<int>& bits) {
    int count = static_cast<int>(bits.size());
    std::vector<double> pulse(count, 0.0);

    double polarity = -1.0;
    int zeros = 0;
    int ones = 0;

    for (int i = 0; i < count; i++) {
        if (bits[i] == 1) {
            pulse[i] = -polarity;
            polarity = pulse[i];

            ones++;
            zeros = 0;
        } else {
            zeros++;

            if (zeros == 4) {
                int start = i - 3;

                if (ones % 2 == 1) {
                    pulse[start + 3] = polarity;
                    polarity = pulse[start + 3];
                } else {
                    double normal = -polarity;

                    pulse[start] = normal;
                    pulse[start + 3] = normal;

                    polarity = normal;
                }

                ones = 0;
                zeros = 0;
            }
        }
    }

    SignalLevels output(count * 2);

    for (int i = 0; i < count; i++) {
        output[2 * i] = pulse[i];
        output[2 * i + 1] = pulse[i];
    }

    return output;
}

std::vector<int> HDB3::decode(const SignalLevels& signal) const {
    int count = bitCount(signal);
    std::vector<double> pulse(count);

    for (int i = 0; i < count; i++) {
        pulse[i] = signal[2 * i];
    }

    std::vector<int> result(count, 0);
    double polarity = -1.0;
    int index = 0;

    while (index < count) {
        if (signValue(pulse[index]) == 0) {
            index++;
            continue;
        }

        if (signValue(pulse[index]) == signValue(polarity)) {
            int start = index - 3;

            if (start >= 0) {
                for (int j = start; j <= index; j++) {
                    result[j] = 0;
                }

                polarity = signValue(pulse[index]);
                index++;
                continue;
            }
        }

        result[index] = 1;
        polarity = signValue(pulse[index]);
        index++;
    }

    return result;
}
