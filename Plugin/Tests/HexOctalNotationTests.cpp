/*
  ==============================================================================

    Hex/Octal Notation Test Suite
    Critical tests for strict left-to-right bit ordering system
    
    PREVENTS REGRESSIONS in the unique LSB-first, digit-reversed notation
    where leftmost bit = LSB and hex digits are processed right-to-left
    during input parsing but displayed left-to-right.

  ==============================================================================
*/

#include "../Source/UPIParser.h"
#include "../Source/PatternUtils.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

class HexOctalNotationTestSuite {
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
    
    void assert_equal(const std::string& test_name, int expected, int actual) {
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
        std::cout << "\n=== Hex/Octal Notation Critical Test Suite ===" << std::endl;
        std::cout << "Testing strict left-to-right bit ordering (LSB-first, digit-reversed)\n" << std::endl;
        
        // Critical round-trip tests
        testHexRoundTrip();
        testOctalRoundTrip();
        
        // Input parsing tests
        testHexInputParsing();
        testOctalInputParsing();
        
        // Display generation tests 
        testHexDisplayGeneration();
        testOctalDisplayGeneration();
        
        // Edge cases and boundary conditions
        testEdgeCases();
        
        // User-provided examples (critical regression prevention)
        testUserExamples();
        
        printSummary();
    }
    
private:
    void testHexRoundTrip() {
        std::cout << "\n--- Hex Round-Trip Tests (Critical) ---" << std::endl;
        
        // CRITICAL: These must work perfectly to prevent regression
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
            {"AB", 8, "11010101", "0xAB"},  // Two nibbles
            {"FF", 8, "11111111", "0xFF"},  // All bits set
            {"00", 8, "00000000", "0x0"},   // No bits set
            
            // Longer patterns
            {"123", 12, "110001000100", "0x123"},
            {"ABC", 12, "110101011010", "0xABC"},
        };
        
        for (const auto& testCase : testCases) {
            // Test input parsing
            auto result = UPIParser::parse("0x" + testCase.input + ":" + std::to_string(testCase.stepCount));
            assert_equal("Hex input 0x" + testCase.input + " valid", true, result.isValid());
            assert_equal("Hex input 0x" + testCase.input + " steps", testCase.stepCount, (int)result.pattern.size());
            assert_equal("Hex input 0x" + testCase.input + " binary", testCase.expectedBinary, patternToString(result.pattern));
            
            // Test display generation
            juce::String displayHex = PatternUtils::getHexString(result.pattern);
            assert_equal("Hex display 0x" + testCase.input, testCase.expectedDisplay, displayHex.toStdString());
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
            
            // User example from message
            {"21", 6, "100010", "o21"},     // Should be reverse of input "12"
        };
        
        for (const auto& testCase : testCases) {
            // Test input parsing  
            auto result = UPIParser::parse("o" + testCase.input + ":" + std::to_string(testCase.stepCount));
            assert_equal("Octal input o" + testCase.input + " valid", true, result.isValid());
            assert_equal("Octal input o" + testCase.input + " steps", testCase.stepCount, (int)result.pattern.size());
            assert_equal("Octal input o" + testCase.input + " binary", testCase.expectedBinary, patternToString(result.pattern));
            
            // Test display generation
            juce::String displayOctal = PatternUtils::getOctalString(result.pattern);
            assert_equal("Octal display o" + testCase.input, testCase.expectedDisplay, displayOctal.toStdString());
        }
    }
    
    void testHexInputParsing() {
        std::cout << "\n--- Hex Input Parsing Tests ---" << std::endl;
        
        // Test the digit reversal logic specifically
        auto result = UPIParser::parse("0x94:8");
        assert_equal("0x94:8 produces tresillo", "10010010", patternToString(result.pattern));
        
        result = UPIParser::parse("0x49:8");  // Reverse of 94
        assert_equal("0x49:8 produces reverse pattern", "10010100", patternToString(result.pattern));
        
        // Test case sensitivity
        result = UPIParser::parse("0xAB:8");
        auto result2 = UPIParser::parse("0xab:8");
        assert_equal("Hex case insensitive", patternToString(result.pattern), patternToString(result2.pattern));
        
        // Test with and without explicit step count
        result = UPIParser::parse("0x94:8");
        result2 = UPIParser::parse("0x94");  // Should auto-calculate steps
        assert_equal("Auto step calculation", 8, (int)result2.pattern.size());
    }
    
    void testOctalInputParsing() {
        std::cout << "\n--- Octal Input Parsing Tests ---" << std::endl;
        
        // Test the digit reversal logic for octal
        auto result = UPIParser::parse("o12:6");
        assert_equal("o12:6 correct pattern", "100010", patternToString(result.pattern));
        
        result = UPIParser::parse("o21:6");  // Reverse of 12
        assert_equal("o21:6 different pattern", "100001", patternToString(result.pattern));
        
        // Test auto step calculation
        result = UPIParser::parse("o12");
        assert_equal("Octal auto steps", 6, (int)result.pattern.size());
    }
    
    void testHexDisplayGeneration() {
        std::cout << "\n--- Hex Display Generation Tests ---" << std::endl;
        
        // Test normal digit order in display (not reversed)
        std::vector<bool> pattern1 = {true, false, false, true, false, false, true, false}; // 10010010
        juce::String display1 = PatternUtils::getHexString(pattern1);
        assert_equal("Tresillo displays as 0x94", "0x94", display1.toStdString());
        
        std::vector<bool> pattern2 = {true, false, false, false, false, false, false, true}; // 10000001
        juce::String display2 = PatternUtils::getHexString(pattern2);
        assert_equal("Pattern displays as 0x81", "0x81", display2.toStdString());
        
        // Test empty pattern
        std::vector<bool> emptyPattern;
        juce::String emptyDisplay = PatternUtils::getHexString(emptyPattern);
        assert_equal("Empty pattern displays as 0x0", "0x0", emptyDisplay.toStdString());
        
        // Test single nibble
        std::vector<bool> singleNibble = {true, false, false, false}; // 1000
        juce::String singleDisplay = PatternUtils::getHexString(singleNibble);
        assert_equal("Single nibble displays as 0x1", "0x1", singleDisplay.toStdString());
    }
    
    void testOctalDisplayGeneration() {
        std::cout << "\n--- Octal Display Generation Tests ---" << std::endl;
        
        // Test normal digit order in display
        std::vector<bool> pattern1 = {true, false, false, false, true, false}; // 100010
        juce::String display1 = PatternUtils::getOctalString(pattern1);
        assert_equal("Pattern displays as o12", "o12", display1.toStdString());
        
        // Test empty pattern
        std::vector<bool> emptyPattern;
        juce::String emptyDisplay = PatternUtils::getOctalString(emptyPattern);
        assert_equal("Empty pattern displays as o0", "o0", emptyDisplay.toStdString());
        
        // Test single group
        std::vector<bool> singleGroup = {true, false, false}; // 100
        juce::String singleDisplay = PatternUtils::getOctalString(singleGroup);
        assert_equal("Single group displays as o1", "o1", singleDisplay.toStdString());
    }
    
    void testEdgeCases() {
        std::cout << "\n--- Edge Cases and Boundary Conditions ---" << std::endl;
        
        // Test maximum hex values
        auto result = UPIParser::parse("0xFFFF:16");
        assert_equal("Max hex valid", true, result.isValid());
        assert_equal("Max hex all bits set", 16, PatternUtils::countOnsets(result.pattern));
        
        // Test maximum octal values
        result = UPIParser::parse("o777:9");
        assert_equal("Max octal valid", true, result.isValid());
        assert_equal("Max octal all bits set", 9, PatternUtils::countOnsets(result.pattern));
        
        // Test partial nibbles/groups
        result = UPIParser::parse("0x1:5");  // 5 bits, partial second nibble
        assert_equal("Partial nibble valid", true, result.isValid());
        assert_equal("Partial nibble steps", 5, (int)result.pattern.size());
        
        result = UPIParser::parse("o1:4");   // 4 bits, partial second group
        assert_equal("Partial octal group valid", true, result.isValid());
        assert_equal("Partial octal group steps", 4, (int)result.pattern.size());
        
        // Test invalid characters
        result = UPIParser::parse("0xGHI:8");
        assert_equal("Invalid hex chars", false, result.isValid());
        
        result = UPIParser::parse("o999:9");
        assert_equal("Invalid octal chars", false, result.isValid());
    }
    
    void testUserExamples() {
        std::cout << "\n--- User-Provided Examples (Regression Prevention) ---" << std::endl;
        
        // These are the exact examples the user provided that must work
        
        // From user message: "0x94:8 should produce 10010010 (tresillo)"
        auto result = UPIParser::parse("0x94:8");
        assert_equal("User example 0x94:8 valid", true, result.isValid());
        assert_equal("User example 0x94:8 binary", "10010010", patternToString(result.pattern));
        juce::String display = PatternUtils::getHexString(result.pattern);
        assert_equal("User example 0x94:8 display", "0x94", display.toStdString());
        
        // From user message: "0x18:8 should be 10000001"
        result = UPIParser::parse("0x18:8");
        assert_equal("User example 0x18:8 valid", true, result.isValid());
        assert_equal("User example 0x18:8 binary", "10000001", patternToString(result.pattern));
        display = PatternUtils::getHexString(result.pattern);
        assert_equal("User example 0x18:8 display", "0x18", display.toStdString());
        
        // From user message: "0x12:8 should be 10000100"
        result = UPIParser::parse("0x12:8");
        assert_equal("User example 0x12:8 valid", true, result.isValid());
        assert_equal("User example 0x12:8 binary", "10000100", patternToString(result.pattern));
        display = PatternUtils::getHexString(result.pattern);
        assert_equal("User example 0x12:8 display", "0x12", display.toStdString());
        
        // From user message: "hex 4 = 0010, not 0100 in binary"
        result = UPIParser::parse("0x4:4");
        assert_equal("User example 0x4:4 valid", true, result.isValid());
        assert_equal("User example 0x4:4 binary", "0010", patternToString(result.pattern));
        display = PatternUtils::getHexString(result.pattern);
        assert_equal("User example 0x4:4 display", "0x4", display.toStdString());
        
        // From user message: "o12:6 gives 100010"
        result = UPIParser::parse("o12:6");
        assert_equal("User example o12:6 valid", true, result.isValid());
        assert_equal("User example o12:6 binary", "100010", patternToString(result.pattern));
        juce::String octalDisplay = PatternUtils::getOctalString(result.pattern);
        assert_equal("User example o12:6 display", "o12", octalDisplay.toStdString());
        
        // Test decimal equivalence
        result = UPIParser::parse("d73:8");  // Should be same as 0x94:8
        assert_equal("Decimal equivalent d73:8", "10010010", patternToString(result.pattern));
    }
    
    void printSummary() {
        std::cout << "\n=== Hex/Octal Test Suite Summary ===" << std::endl;
        std::cout << "Tests Run: " << testsRun << std::endl;
        std::cout << "Passed: " << testsPassed << std::endl;
        std::cout << "Failed: " << testsFailed << std::endl;
        
        if (testsFailed == 0) {
            std::cout << "🎉 ALL HEX/OCTAL TESTS PASSED! 🎉" << std::endl;
            std::cout << "✅ Strict left-to-right bit ordering is working correctly" << std::endl;
            std::cout << "✅ LSB-first, digit-reversed notation is preserved" << std::endl;
        } else {
            std::cout << "❌ " << testsFailed << " tests failed" << std::endl;
            std::cout << "⚠️  CRITICAL: Hex/Octal notation system is BROKEN" << std::endl;
            std::cout << "⚠️  This will cause user confusion and data loss" << std::endl;
            exit(1); // Fail the build
        }
        
        std::cout << "Success rate: " << (100.0 * testsPassed / testsRun) << "%" << std::endl;
    }
};

// Main test runner
int main() {
    HexOctalNotationTestSuite testSuite;
    testSuite.runAllTests();
    return 0;
}