/*
  Test what decimal "2" produces as a pattern
*/

#include <iostream>
#include <vector>

// Simple decimal to binary conversion
std::vector<bool> decimalToPattern(int decimal, int steps = 8) {
    std::vector<bool> pattern(steps, false);
    
    for (int i = 0; i < steps && decimal > 0; ++i) {
        pattern[i] = (decimal & 1) == 1;
        decimal >>= 1;
    }
    
    return pattern;
}

std::string patternToString(const std::vector<bool>& pattern) {
    std::string result;
    for (bool bit : pattern) {
        result += bit ? "1" : "0";
    }
    return result;
}

// Simple pattern combination (OR operation)
std::vector<bool> combinePatterns(const std::vector<bool>& a, const std::vector<bool>& b) {
    size_t maxSize = std::max(a.size(), b.size());
    std::vector<bool> result(maxSize, false);
    
    for (size_t i = 0; i < maxSize; ++i) {
        bool aBit = (i < a.size()) ? a[i] : false;
        bool bBit = (i < b.size()) ? b[i] : false;
        result[i] = aBit || bBit;
    }
    
    return result;
}

int countOnsets(const std::vector<bool>& pattern) {
    return std::count(pattern.begin(), pattern.end(), true);
}

int main() {
    std::cout << "=== TESTING DECIMAL PARSING ===" << std::endl;
    
    // Test what decimal 2 produces
    auto decimal2 = decimalToPattern(2, 8);
    std::cout << "Decimal 2 as pattern: " << patternToString(decimal2) << " (onsets: " << countOnsets(decimal2) << ")" << std::endl;
    
    // E(5,8) pattern
    std::vector<bool> e58 = {true, false, true, true, false, true, true, false}; // 10110110
    std::cout << "E(5,8) pattern:       " << patternToString(e58) << " (onsets: " << countOnsets(e58) << ")" << std::endl;
    
    // Combine them
    auto combined = combinePatterns(e58, decimal2);
    std::cout << "Combined pattern:     " << patternToString(combined) << " (onsets: " << countOnsets(combined) << ")" << std::endl;
    
    std::cout << "\nCompare with reported pattern: 11110110" << std::endl;
    
    return 0;
}