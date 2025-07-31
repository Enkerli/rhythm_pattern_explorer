#include "../Source/PatternUtils.h"
#include <iostream>

std::string patternToString(const std::vector<bool>& pattern) {
    std::string result;
    for (bool bit : pattern) {
        result += bit ? "1" : "0";
    }
    return result;
}

int main() {
    std::cout << "=== Testing Current PatternUtils Barlow Implementation ===" << std::endl;
    
    // Test the specific B(3,8) case
    std::vector<bool> pattern(8, false);
    auto result = PatternUtils::generateBarlowTransformation(pattern, 3, false);
    
    std::cout << "B(3,8) current result: " << patternToString(result) << std::endl;
    std::cout << "Expected: 10001001" << std::endl;
    
    // Test indispensability values for each position
    std::cout << "\nIndispensability values for 8-step pattern:" << std::endl;
    for (int i = 0; i < 8; ++i) {
        double indispensability = PatternUtils::calculateBarlowIndispensability(i, 8);
        std::cout << "Position " << i << ": " << indispensability << std::endl;
    }
    
    return 0;
}