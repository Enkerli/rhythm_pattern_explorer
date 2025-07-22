/*
  Quick test to verify what E(5,8) actually produces
*/

#include <iostream>
#include <vector>

// Simplified test - copy the exact bjorklund implementation from PatternUtils.cpp
std::vector<bool> bjorklundAlgorithm(int beats, int steps) {
    if (beats > steps) beats = steps;
    if (beats <= 0) return std::vector<bool>(steps, false);
    if (beats == steps) return std::vector<bool>(steps, true);
    
    std::vector<bool> pattern;
    std::vector<int> counts;
    std::vector<int> remainders;
    
    int divisor = steps - beats;
    remainders.push_back(beats);
    int level = 0;
    
    do {
        counts.push_back(divisor / remainders[level]);
        remainders.push_back(divisor % remainders[level]);
        divisor = remainders[level];
        level++;
    } while (remainders[level] > 1);
    
    counts.push_back(divisor);
    
    std::function<void(int)> build = [&](int level) {
        if (level == -1) {
            pattern.push_back(false);
        } else if (level == -2) {
            pattern.push_back(true);
        } else {
            for (int i = 0; i < counts[level]; i++) {
                build(level - 1);
            }
            if (remainders[level] != 0) {
                build(level - 2);
            }
        }
    };
    
    build(static_cast<int>(counts.size()) - 1);
    return pattern;
}

std::string patternToString(const std::vector<bool>& pattern) {
    std::string result;
    for (bool bit : pattern) {
        result += bit ? "1" : "0";
    }
    return result;
}

int countOnsets(const std::vector<bool>& pattern) {
    return std::count(pattern.begin(), pattern.end(), true);
}

int main() {
    std::cout << "=== E(5,8) DEBUG TEST ===" << std::endl;
    
    // Test what E(5,8) produces
    auto pattern = bjorklundAlgorithm(5, 8);
    std::cout << "E(5,8): " << patternToString(pattern) << std::endl;
    std::cout << "Onsets: " << countOnsets(pattern) << std::endl;
    std::cout << "Expected: 5 onsets in 8 steps" << std::endl;
    
    // Compare with other patterns
    std::cout << "\nComparison:" << std::endl;
    auto e68 = bjorklundAlgorithm(6, 8);
    std::cout << "E(6,8): " << patternToString(e68) << " (onsets: " << countOnsets(e68) << ")" << std::endl;
    
    // What pattern has 11110110?
    std::vector<bool> mystery = {true, true, true, true, false, true, true, false};
    std::cout << "11110110 has " << countOnsets(mystery) << " onsets" << std::endl;
    
    // Test if 11110110 could be E(5,8) with some transformation
    std::cout << "\nTesting if 11110110 could be a rotation of E(5,8):" << std::endl;
    for (int offset = 0; offset < 8; ++offset) {
        std::vector<bool> rotated(8);
        for (int i = 0; i < 8; ++i) {
            rotated[i] = pattern[(i + offset) % 8];
        }
        std::string rotStr = patternToString(rotated);
        if (rotStr == "11110110") {
            std::cout << "Found match at offset +" << offset << ": " << rotStr << std::endl;
        } else {
            std::cout << "Offset +" << offset << ": " << rotStr << std::endl;
        }
    }
    
    return 0;
}