/*
  ==============================================================================

    Hex/Octal Notation Core Logic Test Suite
    Critical tests for strict left-to-right bit ordering system
    
    Tests the core parsing and display logic without JUCE dependencies
    to ensure the hex/octal notation system works correctly.

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

// Core hex/octal parsing logic (extracted from UPIParser.cpp)
std::vector<bool> parseDecimalCore(int decimal, int stepCount)
{
    std::vector<bool> pattern;
    pattern.reserve(stepCount);
    
    // Convert decimal to binary, LEFT-TO-RIGHT: leftmost bit = LSB
    for (int i = 0; i < stepCount; ++i)
    {
        pattern.push_back((decimal & (1 << i)) != 0);
    }
    
    return pattern;
}

int parseHexWithReversedDigits(const std::string& hexStr)
{
    // Parse hex with reversed digits for left-to-right bit mapping
    int decimal = 0;
    
    // Process hex digits in reverse order
    for (int i = hexStr.length() - 1; i >= 0; --i)
    {
        char c = hexStr[i];
        int hexDigit;
        if (c >= '0' && c <= '9')
            hexDigit = c - '0';
        else if (c >= 'A' && c <= 'F')
            hexDigit = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            hexDigit = c - 'a' + 10;
        else
            continue;
        
        decimal = (decimal << 4) | hexDigit;
    }
    
    return decimal;
}

int parseOctalWithReversedDigits(const std::string& octalStr)
{
    // Parse octal with reversed digits for left-to-right bit mapping
    int decimal = 0;
    
    // Process octal digits in reverse order
    for (int i = octalStr.length() - 1; i >= 0; --i)
    {
        int octalDigit = octalStr[i] - '0';
        decimal = (decimal << 3) | octalDigit;
    }
    
    return decimal;
}

// Core display logic (extracted from PatternUtils.cpp)
std::string getHexStringCore(const std::vector<bool>& pattern)
{
    if (pattern.empty())
        return "0x0";
    
    // Check if pattern is all zeros
    bool allZeros = true;
    for (bool bit : pattern) {
        if (bit) {
            allZeros = false;
            break;
        }
    }
    if (allZeros) return "0x0";
    
    std::string hex;
    int stepCount = pattern.size();
    
    // Process pattern in 4-bit groups from left to right
    for (int groupStart = 0; groupStart < stepCount; groupStart += 4)
    {
        int nibbleValue = 0;
        
        // Process 4 bits in this group
        for (int bitInGroup = 0; bitInGroup < 4 && (groupStart + bitInGroup) < stepCount; ++bitInGroup)
        {
            if (pattern[groupStart + bitInGroup])
            {
                // Left-to-right: each bit position maps directly to bit position in nibble
                nibbleValue |= (1 << bitInGroup);
            }
        }
        
        char hexChar;
        if (nibbleValue < 10)
            hexChar = '0' + nibbleValue;
        else
            hexChar = 'A' + (nibbleValue - 10);
        
        hex += hexChar;
    }
    
    return "0x" + hex;
}

std::string getOctalStringCore(const std::vector<bool>& pattern)
{
    if (pattern.empty())
        return "o0";
    
    // Check if pattern is all zeros
    bool allZeros = true;
    for (bool bit : pattern) {
        if (bit) {
            allZeros = false;
            break;
        }
    }
    if (allZeros) return "o0";
    
    std::string octal;
    int stepCount = pattern.size();
    
    // Process pattern in 3-bit groups from left to right
    for (int groupStart = 0; groupStart < stepCount; groupStart += 3)
    {
        int octalDigit = 0;
        
        // Process 3 bits in this group
        for (int bitInGroup = 0; bitInGroup < 3 && (groupStart + bitInGroup) < stepCount; ++bitInGroup)
        {
            if (pattern[groupStart + bitInGroup])
            {
                // Left-to-right: each bit position maps directly to bit position in octal digit
                octalDigit |= (1 << bitInGroup);
            }
        }
        
        octal += std::to_string(octalDigit);
    }
    
    return "o" + octal;
}

class HexOctalCoreTestSuite {
private:
    int testsRun = 0;
    int testsPassed = 0;
    int testsFailed = 0;
    
    void assert_equal(const std::string& test_name, const std::string& expected, const std::string& actual) {
        testsRun++;
        if (expected == actual) {
            testsPassed++;
            std::cout << "✓ " << test_name << " PASSED" << std::endl;
        } else {
            testsFailed++;
            std::cout << "✗ " << test_name << " FAILED" << std::endl;
            std::cout << "  Expected: " << expected << std::endl;
            std::cout << "  Actual:   " << actual << std::endl;
        }
    }
    
    std::string patternToString(const std::vector<bool>& pattern) {
        std::string result;
        for (bool bit : pattern) {
            result += bit ? "1" : "0";
        }
        return result;
    }

public:
    void runAllTests() {
        std::cout << "\n=== Hex/Octal Core Logic Test Suite ===" << std::endl;
        std::cout << "Testing strict left-to-right bit ordering (LSB-first, digit-reversed)\n" << std::endl;
        
        // Critical round-trip tests
        testHexRoundTrip();
        testOctalRoundTrip();
        
        // User-provided examples (critical regression prevention)
        testUserExamples();
        
        // Edge cases
        testEdgeCases();
        
        printSummary();
        
        // Exit with error code if tests failed (for build integration)
        if (testsFailed > 0) {
            exit(1);
        }
    }
    
private:
    void testHexRoundTrip() {
        std::cout << "\n--- Hex Round-Trip Tests (Critical) ---" << std::endl;
        
        struct TestCase {
            std::string input;
            int stepCount;
            std::string expectedBinary;
            std::string expectedDisplay;
        };
        
        std::vector<TestCase> testCases = {
            // User's critical examples
            {"94", 8, "10010010", "0x94"},  // Tresillo pattern
            {"18", 8, "10000001", "0x18"},  // User example 1
            {"12", 8, "10000100", "0x12"},  // User example 2
            
            // Basic LSB-first tests
            {"1", 4, "1000", "0x1"},        // LSB = 1
            {"8", 4, "0001", "0x8"},        // MSB = 1  
            {"2", 4, "0100", "0x2"},        // Bit 1 = 1
            {"4", 4, "0010", "0x4"},        // Bit 2 = 1
            
            // Multi-nibble tests
            {"AB", 8, "01011101", "0xAB"},  // Two nibbles (reversed digits: B=1101→1101, A=1010→0101)
            {"FF", 8, "11111111", "0xFF"},  // All bits set
            {"00", 8, "00000000", "0x0"},   // No bits set
        };
        
        for (const auto& testCase : testCases) {
            // Test input parsing
            int decimal = parseHexWithReversedDigits(testCase.input);
            auto pattern = parseDecimalCore(decimal, testCase.stepCount);
            assert_equal("Hex input " + testCase.input + " binary", testCase.expectedBinary, patternToString(pattern));
            
            // Test display generation
            std::string displayHex = getHexStringCore(pattern);
            assert_equal("Hex display " + testCase.input, testCase.expectedDisplay, displayHex);
        }
    }
    
    void testOctalRoundTrip() {
        std::cout << "\n--- Octal Round-Trip Tests ---" << std::endl;
        
        struct TestCase {
            std::string input;
            int stepCount;
            std::string expectedBinary;
            std::string expectedDisplay;
        };
        
        std::vector<TestCase> testCases = {
            // Basic 3-bit groups (LSB-first within each group)
            {"1", 3, "100", "o1"},          // LSB = 1
            {"4", 3, "001", "o4"},          // MSB = 1
            {"2", 3, "010", "o2"},          // Middle bit = 1
            {"7", 3, "111", "o7"},          // All bits set
            
            // Multi-digit tests
            {"12", 6, "100010", "o12"},     // Two octal digits
            {"77", 6, "111111", "o77"},     // All bits set
            {"00", 6, "000000", "o0"},      // No bits set
        };
        
        for (const auto& testCase : testCases) {
            // Test input parsing  
            int decimal = parseOctalWithReversedDigits(testCase.input);
            auto pattern = parseDecimalCore(decimal, testCase.stepCount);
            assert_equal("Octal input " + testCase.input + " binary", testCase.expectedBinary, patternToString(pattern));
            
            // Test display generation
            std::string displayOctal = getOctalStringCore(pattern);
            assert_equal("Octal display " + testCase.input, testCase.expectedDisplay, displayOctal);
        }
    }
    
    void testUserExamples() {
        std::cout << "\n--- User-Provided Examples (Regression Prevention) ---" << std::endl;
        
        // These are the exact examples the user provided that must work
        
        // From user: "0x94:8 should produce 10010010 (tresillo)"
        int decimal = parseHexWithReversedDigits("94");
        auto pattern = parseDecimalCore(decimal, 8);
        assert_equal("User example 0x94:8 binary", "10010010", patternToString(pattern));
        std::string display = getHexStringCore(pattern);
        assert_equal("User example 0x94:8 display", "0x94", display);
        
        // From user: "0x18:8 should be 10000001"
        decimal = parseHexWithReversedDigits("18");
        pattern = parseDecimalCore(decimal, 8);
        assert_equal("User example 0x18:8 binary", "10000001", patternToString(pattern));
        display = getHexStringCore(pattern);
        assert_equal("User example 0x18:8 display", "0x18", display);
        
        // From user: "0x12:8 should be 10000100"
        decimal = parseHexWithReversedDigits("12");
        pattern = parseDecimalCore(decimal, 8);
        assert_equal("User example 0x12:8 binary", "10000100", patternToString(pattern));
        display = getHexStringCore(pattern);
        assert_equal("User example 0x12:8 display", "0x12", display);
        
        // From user: "hex 4 = 0010, not 0100 in binary"
        decimal = parseHexWithReversedDigits("4");
        pattern = parseDecimalCore(decimal, 4);
        assert_equal("User example 0x4:4 binary", "0010", patternToString(pattern));
        display = getHexStringCore(pattern);
        assert_equal("User example 0x4:4 display", "0x4", display);
        
        // From user: "o12:6 gives 100010"
        decimal = parseOctalWithReversedDigits("12");
        pattern = parseDecimalCore(decimal, 6);
        assert_equal("User example o12:6 binary", "100010", patternToString(pattern));
        std::string octalDisplay = getOctalStringCore(pattern);
        assert_equal("User example o12:6 display", "o12", octalDisplay);
    }
    
    void testEdgeCases() {
        std::cout << "\n--- Edge Cases ---" << std::endl;
        
        // Test empty pattern
        std::vector<bool> emptyPattern;
        std::string emptyHex = getHexStringCore(emptyPattern);
        assert_equal("Empty pattern hex display", "0x0", emptyHex);
        
        std::string emptyOctal = getOctalStringCore(emptyPattern);
        assert_equal("Empty pattern octal display", "o0", emptyOctal);
        
        // Test single bits
        std::vector<bool> singleBit = {true};
        std::string singleHex = getHexStringCore(singleBit);
        assert_equal("Single bit hex display", "0x1", singleHex);
        
        // Test case insensitivity for hex
        int decimal1 = parseHexWithReversedDigits("AB");
        int decimal2 = parseHexWithReversedDigits("ab");
        auto pattern1 = parseDecimalCore(decimal1, 8);
        auto pattern2 = parseDecimalCore(decimal2, 8);
        assert_equal("Hex case insensitive", patternToString(pattern1), patternToString(pattern2));
    }
    
    void printSummary() {
        std::cout << "\n=== Hex/Octal Core Test Suite Summary ===" << std::endl;
        std::cout << "Tests Run: " << testsRun << std::endl;
        std::cout << "Passed: " << testsPassed << std::endl;
        std::cout << "Failed: " << testsFailed << std::endl;
        
        if (testsFailed == 0) {
            std::cout << "🎉 ALL HEX/OCTAL CORE TESTS PASSED! 🎉" << std::endl;
            std::cout << "✅ Strict left-to-right bit ordering is working correctly" << std::endl;
            std::cout << "✅ LSB-first, digit-reversed notation is preserved" << std::endl;
        } else {
            std::cout << "❌ " << testsFailed << " tests failed" << std::endl;
            std::cout << "⚠️  CRITICAL: Hex/Octal notation system is BROKEN" << std::endl;
            std::cout << "⚠️  This will cause user confusion and data loss" << std::endl;
        }
        
        std::cout << "Success rate: " << (100.0 * testsPassed / testsRun) << "%" << std::endl;
    }
};

// Main test runner
int main() {
    HexOctalCoreTestSuite testSuite;
    testSuite.runAllTests();
    return 0;
}