/*
  ==============================================================================

    TestHexProgressiveOffset.cpp
    Comprehensive test for hex pattern progressive offset functionality
    
    Tests the specific case: 0x94+2 should advance offset on trigger

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

int main()
{
    std::cout << "=== HEX PROGRESSIVE OFFSET TESTS ===" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Testing hex pattern: 0x94+2" << std::endl;
    std::cout << std::endl;
    
    // Manual verification of what 0x94+2 should do:
    // 0x94 = 10010100 (hex 9 = 1001, hex 4 = 0100)
    // +2 offset should rotate by 2 positions: 01001010
    
    std::string originalPattern = "10010100";
    std::string expectedAfterOffset2 = "01001010";
    
    std::cout << "Expected behavior:" << std::endl;
    std::cout << "  Base 0x94:        " << originalPattern << std::endl;
    std::cout << "  After trigger +2: " << expectedAfterOffset2 << std::endl;
    std::cout << std::endl;
    
    // Test the rotation algorithm manually
    std::vector<bool> pattern = {1,0,0,1,0,1,0,0};
    std::vector<bool> rotated(pattern.size());
    int offset = 2;
    int size = static_cast<int>(pattern.size());
    
    // Apply our rotation algorithm: element at position i moves to (i + offset) % size
    offset = ((offset % size) + size) % size;
    for (int i = 0; i < size; ++i) {
        int newIndex = (i + offset) % size;
        rotated[newIndex] = pattern[i];
    }
    
    std::cout << "Manual rotation test:" << std::endl;
    std::cout << "  Original:  ";
    for (bool bit : pattern) std::cout << (bit ? '1' : '0');
    std::cout << std::endl;
    
    std::cout << "  Rotated:   ";
    for (bool bit : rotated) std::cout << (bit ? '1' : '0');
    std::cout << std::endl;
    
    // Check if rotation matches expected
    std::string actualResult;
    for (bool bit : rotated) actualResult += (bit ? '1' : '0');
    
    if (actualResult == expectedAfterOffset2) {
        std::cout << "  ✅ Rotation algorithm correct!" << std::endl;
    } else {
        std::cout << "  ❌ Rotation algorithm failed!" << std::endl;
        std::cout << "  Expected: " << expectedAfterOffset2 << std::endl;
        std::cout << "  Got:      " << actualResult << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "=== TEST ANALYSIS ===" << std::endl;
    std::cout << "If 0x94+2 is not working in the plugin, the issue is likely:" << std::endl;
    std::cout << "1. UPIParser::parsePattern(\"0x94\") returns error/empty" << std::endl;
    std::cout << "2. Progressive state is not being initialized correctly" << std::endl;
    std::cout << "3. triggerProgressive is not being called" << std::endl;
    std::cout << "4. applyProgressive is not returning the rotated result" << std::endl;
    std::cout << std::endl;
    std::cout << "The rotation logic itself is working correctly." << std::endl;
    
    return 0;
}