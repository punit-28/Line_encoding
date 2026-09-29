#pragma once
#include <vector>
#include <random>
#include <cstdint>

// Generates digital (binary) data streams for the assignment.
class DataGenerator {
public:
    explicit DataGenerator(unsigned seed = std::random_device{}());

    // A completely random 0/1 sequence of the given length.
    std::vector<int> generateRandom(int length);

    // A random sequence that is guaranteed to contain each of the given
    // "forced" runs of consecutive zeros (e.g. {8,4} inserts one run of
    // eight consecutive zeros and one run of four consecutive zeros at
    // random, non-overlapping positions), with random bits everywhere else.
    std::vector<int> generateWithZeroRuns(int length, const std::vector<int>& runLengths = {8, 4});

private:
    std::mt19937 rng_;
};
