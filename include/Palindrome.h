#pragma once
#include <vector>
#include <string>
#include <utility>

// Finds the longest palindromic substring/subsequence-of-consecutive-bits
// in a 0/1 data stream using Manacher's algorithm, O(n) time and space.
// (A naive centre-expansion approach is O(n^2); Manacher's is the
// competitive-coding-friendly choice.)
namespace Palindrome {

    // Returns {startIndex, length} of the longest palindromic contiguous
    // run within `data`.
    std::pair<int, int> longestPalindrome(const std::vector<int>& data);

    // Convenience: extracts the actual palindromic sub-sequence of bits.
    std::vector<int> extract(const std::vector<int>& data);

} // namespace Palindrome
