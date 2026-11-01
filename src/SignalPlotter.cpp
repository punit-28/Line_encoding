
#include "SignalPlotter.h"
#include <fstream>
#include <algorithm>
#include <cmath>

namespace {
    const int WIDTH = 900;
    const int HEIGHT = 260;
    const int MARGIN = 40;
}

namespace SignalPlotter {

void plotSignal(const SignalLevels& signal,
               const std::string& filePath,
               const std::string& title) {

    std::ofstream file(filePath);

    int total = static_cast<int>(signal.size());
    int bitTotal = total / 2;

    double spacing = (WIDTH - 2.0 * MARGIN) / std::max(1, bitTotal);
    double center = HEIGHT / 2.0;
    double amplitude = HEIGHT / 2.0 - MARGIN / 2.0;

    file << "<svg xmlns='http://www.w3.org/2000/svg' "
         << "width='" << WIDTH << "' height='" << HEIGHT << "'>\n";

    file << "<rect width='100%' height='100%' fill='white'/>\n";

    file << "<text x='" << MARGIN << "' y='20' "
         << "font-family='monospace' font-size='14'>"
         << title << "</text>\n";

    file << "<line x1='" << MARGIN << "' y1='" << center
         << "' x2='" << WIDTH - MARGIN << "' y2='" << center
         << "' stroke='#cccccc' stroke-width='1'/>\n";

    file << "<polyline fill='none' stroke='#2563eb' "
         << "stroke-width='2' points='";

    for (int i = 0; i < total; i++) {
        double startX = MARGIN + i * spacing / 2.0;
        double endX = MARGIN + (i + 1) * spacing / 2.0;
        double yPosition = center - signal[i] * amplitude;

        file << startX << "," << yPosition << " "
             << endX << "," << yPosition << " ";
    }

    file << "'/>\n";

    // Draw bit separation lines
    for (int i = 0; i <= bitTotal; i++) {
        double xPosition = MARGIN + i * spacing;

        file << "<line x1='" << xPosition
             << "' y1='" << center - amplitude - 5
             << "' x2='" << xPosition
             << "' y2='" << center + amplitude + 5
             << "' stroke='#eeeeee' stroke-width='1'/>\n";
    }

    file << "</svg>\n";
}


void plotStaircase(const std::vector<double>& original,
                   const std::vector<double>& staircase,
                   const std::string& filePath,
                   const std::string& title) {

    std::ofstream file(filePath);

    size_t totalPoints = std::max(original.size(), staircase.size());

    double minimum = 0.0;
    double maximum = 0.0;
    bool initialized = false;

    auto updateRange = [&](double value) {
        if (!initialized) {
            minimum = maximum = value;
            initialized = true;
        }

        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
    };

    for (double value : original) {
        updateRange(value);
    }

    for (double value : staircase) {
        updateRange(value);
    }

    if (!initialized) {
        minimum = -1.0;
        maximum = 1.0;
    }

    if (maximum - minimum < 1e-9) {
        maximum += 1.0;
        minimum -= 1.0;
    }

    double spacing = (WIDTH - 2.0 * MARGIN) /
        std::max<size_t>(1, totalPoints > 1 ? totalPoints - 1 : 1);

    auto getY = [&](double value) {
        double ratio = (value - minimum) / (maximum - minimum);

        return (HEIGHT - MARGIN) -
               ratio * (HEIGHT - 2.0 * MARGIN);
    };

    file << "<svg xmlns='http://www.w3.org/2000/svg' "
         << "width='" << WIDTH << "' height='" << HEIGHT << "'>\n";

    file << "<rect width='100%' height='100%' fill='white'/>\n";

    file << "<text x='" << MARGIN << "' y='20' "
         << "font-family='monospace' font-size='14'>"
         << title << "</text>\n";


    // Plot original signal
    if (!original.empty()) {

        file << "<polyline fill='none' stroke='#f97316' "
             << "stroke-width='2' points='";

        for (size_t i = 0; i < original.size(); i++) {
            double xPosition = MARGIN + i * spacing;

            file << xPosition << "," << getY(original[i]) << " ";
        }

        file << "'/>\n";
    }


    // Plot reconstructed staircase
    if (!staircase.empty()) {

        file << "<polyline fill='none' stroke='#2563eb' "
             << "stroke-width='2' points='";

        for (size_t i = 0; i < staircase.size(); i++) {
            double currentX = MARGIN + i * spacing;
            double nextX = MARGIN + (i + 1) * spacing;
            double currentY = getY(staircase[i]);

            file << currentX << "," << currentY << " "
                 << nextX << "," << currentY << " ";
        }

        file << "'/>\n";
    }

    file << "<text x='" << WIDTH - 220
         << "' y='20' font-family='monospace' "
         << "font-size='12' fill='#f97316'>original</text>\n";

    file << "<text x='" << WIDTH - 220
         << "' y='36' font-family='monospace' "
         << "font-size='12' fill='#2563eb'>"
         << "decoder staircase</text>\n";

    file << "</svg>\n";
}

} // namespace SignalPlotter
