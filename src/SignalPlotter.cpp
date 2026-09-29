#include "SignalPlotter.h"
#include <fstream>
#include <algorithm>
#include <cmath>

namespace {
const int kWidth = 900;
const int kHeight = 260;
const int kMargin = 40;
}

namespace SignalPlotter {

void plotSignal(const SignalLevels& signal, const std::string& filePath, const std::string& title) {
    std::ofstream out(filePath);
    int n = static_cast<int>(signal.size()); // half-bit samples
    int bits = n / 2;
    double xStep = (kWidth - 2.0 * kMargin) / std::max(1, bits);
    double midY = kHeight / 2.0;
    double amp = (kHeight / 2.0 - kMargin / 2.0);

    out << "<svg xmlns='http://www.w3.org/2000/svg' width='" << kWidth << "' height='" << kHeight << "'>\n";
    out << "<rect width='100%' height='100%' fill='white'/>\n";
    out << "<text x='" << kMargin << "' y='20' font-family='monospace' font-size='14'>" << title << "</text>\n";
    out << "<line x1='" << kMargin << "' y1='" << midY << "' x2='" << (kWidth - kMargin) << "' y2='" << midY
        << "' stroke='#cccccc' stroke-width='1'/>\n";

    out << "<polyline fill='none' stroke='#2563eb' stroke-width='2' points='";
    for (int i = 0; i < n; ++i) {
        double x0 = kMargin + i * (xStep / 2.0);
        double x1 = kMargin + (i + 1) * (xStep / 2.0);
        double y = midY - signal[i] * amp;
        out << x0 << "," << y << " " << x1 << "," << y << " ";
    }
    out << "'/>\n";

    // vertical bit-boundary ticks
    for (int i = 0; i <= bits; ++i) {
        double x = kMargin + i * xStep;
        out << "<line x1='" << x << "' y1='" << (midY - amp - 5) << "' x2='" << x << "' y2='" << (midY + amp + 5)
            << "' stroke='#eeeeee' stroke-width='1'/>\n";
    }
    out << "</svg>\n";
}

void plotStaircase(const std::vector<double>& original,
                    const std::vector<double>& staircase,
                    const std::string& filePath,
                    const std::string& title) {
    std::ofstream out(filePath);
    size_t n = std::max(original.size(), staircase.size());
    double allMin = 0, allMax = 0;
    bool first = true;
    auto consider = [&](double v) {
        if (first) { allMin = allMax = v; first = false; }
        allMin = std::min(allMin, v);
        allMax = std::max(allMax, v);
    };
    for (double v : original) consider(v);
    for (double v : staircase) consider(v);
    if (allMax - allMin < 1e-9) { allMax += 1; allMin -= 1; }

    double xStep = (kWidth - 2.0 * kMargin) / std::max<size_t>(1, n - 1 > 0 ? n - 1 : 1);
    auto yOf = [&](double v) {
        double t = (v - allMin) / (allMax - allMin);
        return (kHeight - kMargin) - t * (kHeight - 2.0 * kMargin);
    };

    out << "<svg xmlns='http://www.w3.org/2000/svg' width='" << kWidth << "' height='" << kHeight << "'>\n";
    out << "<rect width='100%' height='100%' fill='white'/>\n";
    out << "<text x='" << kMargin << "' y='20' font-family='monospace' font-size='14'>" << title << "</text>\n";

    if (!original.empty()) {
        out << "<polyline fill='none' stroke='#f97316' stroke-width='2' points='";
        for (size_t i = 0; i < original.size(); ++i) {
            out << (kMargin + i * xStep) << "," << yOf(original[i]) << " ";
        }
        out << "'/>\n";
    }
    if (!staircase.empty()) {
        // step (staircase) rendering: horizontal then vertical segments
        out << "<polyline fill='none' stroke='#2563eb' stroke-width='2' points='";
        for (size_t i = 0; i < staircase.size(); ++i) {
            double x = kMargin + i * xStep;
            double xNext = kMargin + (i + 1) * xStep;
            double y = yOf(staircase[i]);
            out << x << "," << y << " " << xNext << "," << y << " ";
        }
        out << "'/>\n";
    }
    out << "<text x='" << (kWidth - 220) << "' y='20' font-family='monospace' font-size='12' fill='#f97316'>original</text>\n";
    out << "<text x='" << (kWidth - 220) << "' y='36' font-family='monospace' font-size='12' fill='#2563eb'>decoder staircase</text>\n";
    out << "</svg>\n";
}

} // namespace SignalPlotter
