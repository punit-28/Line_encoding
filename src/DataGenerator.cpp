#include "DataGenerator.h"
#include <algorithm>
#include <numeric>

DataGenerator::DataGenerator(unsigned seed) : rng_(seed) {}

std::vector<int> DataGenerator::generateRandom(int length) {
    std::uniform_int_distribution<int> bit(0, 1);
    std::vector<int> data(length);
    for (int i = 0; i < length; ++i) data[i] = bit(rng_);
    return data;
}

std::vector<int> DataGenerator::generateWithZeroRuns(int length, const std::vector<int>& runLengths) {
    std::vector<int> data = generateRandom(length);

    // Figure out how much room the forced runs need in total.
    int totalRunSpace = std::accumulate(runLengths.begin(), runLengths.end(), 0);
    if (totalRunSpace > length) return data; // not enough room, fall back to pure random

    // Pick non-overlapping starting positions for each forced run,
    // spacing them out across the available length.
    // here is we use cursor to track the current position in the data vector, and remainingSlack to determine how much space we have left to distribute gaps between runs. We use a uniform distribution to randomly select gaps between runs, ensuring that they do not overlap and fit within the total length of the data vector.
    int cursor = 0;
    int remainingSlack = length - totalRunSpace;
    std::uniform_int_distribution<int> gapDist(0, std::max(1, remainingSlack / static_cast<int>(runLengths.size() + 1)));

    for (int runLen : runLengths) {
        int gap = gapDist(rng_);
        cursor = std::min(cursor + gap, length - runLen);
        if (cursor < 0) cursor = 0;
        for (int i = 0; i < runLen && cursor + i < length; ++i) {
            data[cursor + i] = 0;
        }
        cursor += runLen;
    }
    return data;
}
