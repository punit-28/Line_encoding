
#include "Palindrome.h"

namespace Palindrome {

std::pair<int, int> longestPalindrome(const std::vector<int>& data) {

    int size = static_cast<int>(data.size());

    if (size == 0) {
        return {0, 0};
    }

    // Prepare the input by inserting separator values.
    std::vector<int> transformed;
    transformed.reserve(2 * size + 3);

    transformed.push_back(3);
    transformed.push_back(2);

    for (int value : data) {
        transformed.push_back(value);
        transformed.push_back(2);
    }

    transformed.push_back(4);

    int total = static_cast<int>(transformed.size());

    std::vector<int> radius(total, 0);

    int middle = 0;
    int boundary = 0;

    int maximumCenter = 0;
    int maximumLength = 0;

    for (int pos = 1; pos < total - 1; pos++) {

        if (pos < boundary) {
            int opposite = 2 * middle - pos;

            radius[pos] = std::min(
                boundary - pos, radius[opposite]
            );
        }

        // Expand around the current center.
        while (transformed[pos + radius[pos] + 1] ==
               transformed[pos - radius[pos] - 1]) {

            radius[pos]++;
        }

        // Update the rightmost palindrome boundary.
        if (pos + radius[pos] > boundary) {
            middle = pos;
            boundary = pos + radius[pos];
        }

        // Store the longest palindrome found so far.
        if (radius[pos] > maximumLength) {
            maximumLength = radius[pos];
            maximumCenter = pos;
        }
    }

    int firstIndex = (maximumCenter - maximumLength) / 2;
    int palindromeSize = maximumLength;

    return {firstIndex, palindromeSize};
}


std::vector<int> extract(const std::vector<int>& data) {

    auto [firstIndex, palindromeSize] = longestPalindrome(data);

    if (palindromeSize == 0) {
        return {};
    }

    auto first = data.begin() + firstIndex;
    auto last = first + palindromeSize;

    return std::vector<int>(first, last);
}

} // namespace Palindrome
