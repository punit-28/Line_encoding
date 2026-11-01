
#include "DataGenerator.h"
#include <algorithm>
#include <numeric>

DataGenerator::DataGenerator(unsigned seed) : rng_(seed) {}

std::vector<int> DataGenerator::generateRandom(int length) {
    std::vector<int> result;
    result.reserve(length);

    std::uniform_int_distribution<int> randomBit(0, 1);

    for (int index = 0; index < length; index++) {
        result.push_back(randomBit(rng_));
    }

    return result;
}

std::vector<int> DataGenerator::generateWithZeroRuns(
    int length, const std::vector<int>& runLengths) {

    std::vector<int> result = generateRandom(length);

    int requiredSpace = std::accumulate(
        runLengths.begin(), runLengths.end(), 0
    );

    if (requiredSpace > length) {
        return result;
    }

    int position = 0;
    int freeSpace = length - requiredSpace;

    std::uniform_int_distribution<int> randomGap(
        0, std::max(1, freeSpace / static_cast<int>(runLengths.size() + 1))
    );

    for (const int currentRun : runLengths) {
        int offset = randomGap(rng_);

        position = std::min(position + offset, length - currentRun);
        position = std::max(0, position);

        for (int j = 0; j < currentRun && position + j < length; j++) {
            result[position + j] = 0;
        }

        position += currentRun;
    }

    return result;
}
