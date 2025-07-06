/*
  ==============================================================================

    Comprehensive UPI Parser Test Suite
    Tests all pattern parsing functionality after recent changes

  ==============================================================================
*/

#include "../Source/UPIParser.h"
#include "../Source/PatternEngine.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

class UPIParserTestSuite {
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
    
    std::string patternToHex(const std::vector<bool>& pattern) {
        if (pattern.empty()) return "0x0";
        
        uint64_t value = 0;
        for (size_t i = 0; i < pattern.size() && i < 64; ++i) {
            if (pattern[i]) {
                value |= (1ULL << i);
            }
        }
        
        char hexStr[32];
        snprintf(hexStr, sizeof(hexStr), "0x%llX", value);
        return std::string(hexStr);
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
        std::cout << "\n=== UPI Parser Comprehensive Test Suite ===" << std::endl;
        std::cout << "Testing all pattern parsing after recent changes\n" << std::endl;
        
        // Basic pattern tests
        testBasicPatterns();
        
        // Polygon combination tests (recent fix)
        testPolygonCombinations();
        
        // Progressive offset tests
        testProgressiveOffsets();
        
        // Scene cycling tests
        testSceneCycling();
        
        // Edge cases and error handling
        testEdgeCases();
        
        // Cross-validation with webapp patterns
        testWebappConsistency();
        
        printSummary();
    }
    
private:
    void testBasicPatterns() {
        std::cout << "\n--- Basic Pattern Tests ---" << std::endl;
        
        // Single polygons
        auto result = UPIParser::parse("P(3,0)");
        assert_equal("P(3,0) valid", true, result.isValid());
        assert_equal("P(3,0) steps", 3, (int)result.pattern.size());
        assert_equal("P(3,0) binary", "101", patternToString(result.pattern));
        
        result = UPIParser::parse("P(7,0)");
        assert_equal("P(7,0) valid", true, result.isValid());
        assert_equal("P(7,0) steps", 7, (int)result.pattern.size());
        assert_equal("P(7,0) binary", "1111111", patternToString(result.pattern));
        
        result = UPIParser::parse("P(7,0,2)");
        assert_equal("P(7,0,2) valid", true, result.isValid());
        assert_equal("P(7,0,2) steps", 14, (int)result.pattern.size());
        assert_equal("P(7,0,2) binary", "10101010101010", patternToString(result.pattern));
        
        result = UPIParser::parse("P(11,0,2)");
        assert_equal("P(11,0,2) valid", true, result.isValid());
        assert_equal("P(11,0,2) hex", "0x2AAAAA", patternToHex(result.pattern));
        
        // Euclidean patterns
        result = UPIParser::parse("E(3,8)");
        assert_equal("E(3,8) valid", true, result.isValid());
        assert_equal("E(3,8) steps", 8, (int)result.pattern.size());
        assert_equal("E(3,8) hex", "0x92", patternToHex(result.pattern)); // Tresillo
        
        result = UPIParser::parse("E(5,8)");
        assert_equal("E(5,8) valid", true, result.isValid());
        assert_equal("E(5,8) hex", "0xB5", patternToHex(result.pattern)); // Cinquillo
    }
    
    void testPolygonCombinations() {
        std::cout << "\n--- Polygon Combination Tests (Recent Fix) ---" << std::endl;
        
        // The main fix: P(7,0)+P(11,0) should produce 77-step pattern
        auto result = UPIParser::parse("P(7,0)+P(11,0)");
        assert_equal("P(7,0)+P(11,0) valid", true, result.isValid());
        assert_equal("P(7,0)+P(11,0) steps", 77, (int)result.pattern.size()); // LCM(7,11) = 77
        
        // Should NOT be detected as progressive offset anymore
        std::string hex = patternToHex(result.pattern);
        assert_equal("P(7,0)+P(11,0) not simple", false, hex == "0x7F"); // Should NOT be 1111111
        
        // Multi-polygon combinations
        result = UPIParser::parse("P(3,1)+P(5,0)");
        assert_equal("P(3,1)+P(5,0) valid", true, result.isValid());
        assert_equal("P(3,1)+P(5,0) steps", 15, (int)result.pattern.size()); // LCM(3,5) = 15
        
        result = UPIParser::parse("P(3,1)+P(5,0)+P(2,0)");
        assert_equal("P(3,1)+P(5,0)+P(2,0) valid", true, result.isValid());
        assert_equal("P(3,1)+P(5,0)+P(2,0) steps", 30, (int)result.pattern.size()); // LCM(3,5,2) = 30
        
        // Combinations with expansion
        result = UPIParser::parse("P(3,0,2)+P(5,0,2)");
        assert_equal("P(3,0,2)+P(5,0,2) valid", true, result.isValid());
        assert_equal("P(3,0,2)+P(5,0,2) steps", 30, (int)result.pattern.size()); // LCM(6,10) = 30
    }
    
    void testProgressiveOffsets() {
        std::cout << "\n--- Progressive Offset Tests ---" << std::endl;
        
        // These should be detected as progressive offset (number after +)
        auto result = UPIParser::parse("P(7,0)+2");
        assert_equal("P(7,0)+2 valid", true, result.isValid());
        // This should trigger progressive offset logic, not combination
        
        result = UPIParser::parse("E(3,8)+1");
        assert_equal("E(3,8)+1 valid", true, result.isValid());
        
        result = UPIParser::parse("P(5,0)+3");
        assert_equal("P(5,0)+3 valid", true, result.isValid());
        
        // Progressive lengthening
        result = UPIParser::parse("P(3,0)*2");
        assert_equal("P(3,0)*2 valid", true, result.isValid());
        
        result = UPIParser::parse("E(3,8)*3");
        assert_equal("E(3,8)*3 valid", true, result.isValid());
    }
    
    void testSceneCycling() {
        std::cout << "\n--- Scene Cycling Tests ---" << std::endl;
        
        // Basic scene cycling
        auto result = UPIParser::parse("P(3,0)|E(5,8)|P(7,1)");
        assert_equal("Basic scenes valid", true, result.isValid());
        
        // Scenes with progressive offsets
        result = UPIParser::parse("P(3,0)+1|P(5,0)+2|E(3,8)+1");
        assert_equal("Progressive scenes valid", true, result.isValid());
        
        // Scenes with combinations
        result = UPIParser::parse("P(3,0)+P(5,0)|E(3,8)|P(7,0)+P(11,0)");
        assert_equal("Combination scenes valid", true, result.isValid());
        
        // Scenes with lengthening
        result = UPIParser::parse("P(3,0)*2|E(5,8)*3|P(7,0)*1");
        assert_equal("Lengthening scenes valid", true, result.isValid());
    }
    
    void testEdgeCases() {
        std::cout << "\n--- Edge Cases and Error Handling ---" << std::endl;
        
        // Empty input
        auto result = UPIParser::parse("");
        assert_equal("Empty input invalid", false, result.isValid());
        
        // Invalid syntax
        result = UPIParser::parse("P(");
        assert_equal("Invalid syntax invalid", false, result.isValid());
        
        result = UPIParser::parse("P(3,0");
        assert_equal("Unclosed paren invalid", false, result.isValid());
        
        // Large patterns
        result = UPIParser::parse("P(32,0)");
        assert_equal("P(32,0) valid", true, result.isValid());
        assert_equal("P(32,0) steps", 32, (int)result.pattern.size());
        
        // Invalid polygon size
        result = UPIParser::parse("P(1,0)");
        assert_equal("P(1,0) invalid", false, result.isValid());
        
        result = UPIParser::parse("P(65,0)");
        assert_equal("P(65,0) invalid", false, result.isValid());
        
        // Hex patterns
        result = UPIParser::parse("0x92:8");
        assert_equal("0x92:8 valid", true, result.isValid());
        assert_equal("0x92:8 steps", 8, (int)result.pattern.size());
        assert_equal("0x92:8 hex", "0x92", patternToHex(result.pattern));
        
        // Binary patterns
        result = UPIParser::parse("10010010");
        assert_equal("Binary valid", true, result.isValid());
        assert_equal("Binary steps", 8, (int)result.pattern.size());
        assert_equal("Binary hex", "0x92", patternToHex(result.pattern));
    }
    
    void testWebappConsistency() {
        std::cout << "\n--- Webapp Consistency Tests ---" << std::endl;
        
        // Test patterns that should match webapp exactly
        auto result = UPIParser::parse("E(3,8)");
        assert_equal("Tresillo consistency", "0x92", patternToHex(result.pattern));
        
        result = UPIParser::parse("E(5,8)");
        assert_equal("Cinquillo consistency", "0xB5", patternToHex(result.pattern));
        
        result = UPIParser::parse("P(3,0)");
        assert_equal("Triangle consistency", "0x5", patternToHex(result.pattern)); // 101
        
        result = UPIParser::parse("P(4,0)");
        assert_equal("Square consistency", "0x9", patternToHex(result.pattern)); // 1001
        
        result = UPIParser::parse("P(5,0)");
        assert_equal("Pentagon consistency", "0x11", patternToHex(result.pattern)); // 10001
        
        // The critical test: polygon combination
        result = UPIParser::parse("P(7,0)+P(11,0)");
        // Expected: 77-step pattern matching webapp 0x102240C10A0508000000:77
        assert_equal("P(7,0)+P(11,0) consistency", 77, (int)result.pattern.size());
        
        // Verify it's not the broken 7-step pattern
        std::string hexResult = patternToHex(result.pattern);
        assert_equal("P(7,0)+P(11,0) not broken", false, hexResult == "0x7F");
    }
    
    void printSummary() {
        std::cout << "\n=== Test Suite Summary ===" << std::endl;
        std::cout << "Tests Run: " << testsRun << std::endl;
        std::cout << "Passed: " << testsPassed << std::endl;
        std::cout << "Failed: " << testsFailed << std::endl;
        
        if (testsFailed == 0) {
            std::cout << "🎉 ALL TESTS PASSED! 🎉" << std::endl;
        } else {
            std::cout << "❌ " << testsFailed << " tests failed" << std::endl;
        }
        
        std::cout << "Success rate: " << (100.0 * testsPassed / testsRun) << "%" << std::endl;
    }
};

// Main test runner
int main() {
    UPIParserTestSuite testSuite;
    testSuite.runAllTests();
    return 0;
}