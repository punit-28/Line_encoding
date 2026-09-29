#include "Palindrome.h"

namespace Palindrome {

std::pair<int, int> longestPalindrome(const std::vector<int>& data) {
    int n = static_cast<int>(data.size());
    if (n == 0) return {0, 0};

    // Transform: insert separators so every palindrome (odd/even) becomes odd length.
    // '#' cannot equal 0 or 1, so we use the sentinel value 2.
    std::vector<int> t;
    t.reserve(2 * n + 3);
    t.push_back(3); // start sentinel (distinct from '#'=2 and bits 0/1)
    t.push_back(2);
    for (int v : data) {
        t.push_back(v);
        t.push_back(2);
    }
    t.push_back(4); // end sentinel

    int m = static_cast<int>(t.size());
    std::vector<int> p(m, 0); // radius array
    int center = 0, right = 0;
    int bestCenter = 0, bestRadius = 0;

    for (int i = 1; i < m - 1; ++i) {
        if (i < right) {
            int mirror = 2 * center - i;
            p[i] = std::min(right - i, p[mirror]);
        }
        while (t[i + p[i] + 1] == t[i - p[i] - 1]) {
            ++p[i];
        }
        if (i + p[i] > right) {
            center = i;
            right = i + p[i];
        }
        if (p[i] > bestRadius) {
            bestRadius = p[i];
            bestCenter = i;
        }
    }

    // Map back to original indices.
    int start = (bestCenter - bestRadius) / 2;
    int length = bestRadius;
    return {start, length};
}

std::vector<int> extract(const std::vector<int>& data) {
    auto [start, length] = longestPalindrome(data);
    if (length <= 0) return {};
    return std::vector<int>(data.begin() + start, data.begin() + start + length);
}

} // namespace Palindrome
