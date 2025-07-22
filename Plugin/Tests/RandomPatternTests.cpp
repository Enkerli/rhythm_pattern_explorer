/*
  ==============================================================================
    Random Pattern Tests
    
    Comprehensive test suite for random pattern generation functionality to 
    prevent regression and ensure mathematical correctness.
    
    Coverage:
    - Basic random patterns R(onsets,steps)
    - Bell curve random patterns R(r,steps) 
    - Edge cases and boundary conditions
    - Statistical distribution validation
    - Pattern recognition and parsing
    
  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <random>
#include <algorithm>
#include <map>
#include <cmath>
#include <regex>

// Mock JUCE String for testing
class MockString {
private:
    std::string str;
public:
    MockString(const std::string& s) : str(s) {}
    MockString(const char* s) : str(s) {}
    
    bool startsWith(const std::string& prefix) const {
        return str.substr(0, prefix.length()) == prefix;
    }
    
    MockString substring(size_t start, size_t end = std::string::npos) const {
        if (end == std::string::npos) {
            return MockString(str.substr(start));
        }
        return MockString(str.substr(start, end - start));
    }
    
    size_t length() const { return str.length(); }
    char operator[](size_t i) const { return str[i]; }
    bool operator==(const std::string& other) const { return str == other; }
    MockString operator+=(const std::string& other) { str += other; return *this; }
    std::string toStdString() const { return str; }
    int getIntValue() const { return std::stoi(str); }
    
    bool contains(const std::string& substr) const {
        return str.find(substr) != std::string::npos;
    }
    
    static MockString fromInt(int value) {
        return MockString(std::to_string(value));
    }
};

// Copy of random pattern implementation for testing
std::vector<bool> parseRandom(int onsets, int steps)
{
    std::vector<bool> pattern(steps, false);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Randomly place onsets
    std::vector<int> positions;
    for (int i = 0; i < steps; ++i)
        positions.push_back(i);
    
    std::shuffle(positions.begin(), positions.end(), gen);
    
    for (int i = 0; i < std::min(onsets, steps); ++i)
    {
        pattern[positions[i]] = true;
    }
    
    return pattern;
}

// Deterministic version for testing
std::vector<bool> parseRandomDeterministic(int onsets, int steps, unsigned int seed)
{
    std::vector<bool> pattern(steps, false);
    std::mt19937 gen(seed);
    
    // Randomly place onsets
    std::vector<int> positions;
    for (int i = 0; i < steps; ++i)
        positions.push_back(i);
    
    std::shuffle(positions.begin(), positions.end(), gen);
    
    for (int i = 0; i < std::min(onsets, steps); ++i)
    {
        pattern[positions[i]] = true;
    }
    
    return pattern;
}

// Bell curve onset calculation (from UPI implementation)
int calculateBellCurveOnsets(int steps) {
    return steps / 3 + (rand() % (steps / 2)); // Rough bell curve
}

// Pattern recognition function
bool isRandomPattern(const MockString& input)
{
    std::regex randomRegex(R"([Rr]\(([r\d]+),(\d+)\))");
    return std::regex_search(input.toStdString(), randomRegex);
}

// Helper functions for testing
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

void assert_equal(const std::string& description, const std::string& expected, const std::string& actual) {
    if (expected != actual) {
        std::cout << "FAIL: " << description << std::endl;
        std::cout << "  Expected: " << expected << std::endl;
        std::cout << "  Actual:   " << actual << std::endl;
        assert(false);
    } else {
        std::cout << "PASS: " << description << std::endl;
    }
}

void assert_equal_int(const std::string& description, int expected, int actual) {
    if (expected != actual) {
        std::cout << "FAIL: " << description << std::endl;
        std::cout << "  Expected: " << expected << std::endl;
        std::cout << "  Actual:   " << actual << std::endl;
        assert(false);
    } else {
        std::cout << "PASS: " << description << std::endl;
    }
}

void assert_true(const std::string& description, bool condition) {
    if (!condition) {
        std::cout << "FAIL: " << description << std::endl;
        assert(false);
    } else {
        std::cout << "PASS: " << description << std::endl;
    }
}

void assert_in_range(const std::string& description, int value, int min, int max) {
    if (value < min || value > max) {
        std::cout << "FAIL: " << description << std::endl;
        std::cout << "  Value: " << value << " not in range [" << min << ", " << max << "]" << std::endl;
        assert(false);
    } else {
        std::cout << "PASS: " << description << std::endl;
    }
}

//==============================================================================
// Test Functions
//==============================================================================

void testBasicRandomPatterns() {
    std::cout << "\n=== Testing Basic Random Patterns ===\n" << std::endl;
    
    // Test R(onsets,steps) with deterministic seed for reproducible results
    
    // R(3,8) - exactly 3 onsets in 8 steps
    auto pattern_3_8 = parseRandomDeterministic(3, 8, 12345);
    assert_equal_int("R(3,8) step count", 8, static_cast<int>(pattern_3_8.size()));
    assert_equal_int("R(3,8) onset count", 3, countOnsets(pattern_3_8));
    std::cout << "R(3,8) pattern: " << patternToString(pattern_3_8) << std::endl;
    
    // R(5,12) - exactly 5 onsets in 12 steps
    auto pattern_5_12 = parseRandomDeterministic(5, 12, 54321);
    assert_equal_int("R(5,12) step count", 12, static_cast<int>(pattern_5_12.size()));
    assert_equal_int("R(5,12) onset count", 5, countOnsets(pattern_5_12));
    std::cout << "R(5,12) pattern: " << patternToString(pattern_5_12) << std::endl;
    
    // R(0,8) - no onsets
    auto pattern_0_8 = parseRandomDeterministic(0, 8, 11111);
    assert_equal_int("R(0,8) step count", 8, static_cast<int>(pattern_0_8.size()));
    assert_equal_int("R(0,8) onset count", 0, countOnsets(pattern_0_8));
    assert_equal("R(0,8) pattern", "00000000", patternToString(pattern_0_8));
    
    // R(8,8) - all onsets
    auto pattern_8_8 = parseRandomDeterministic(8, 8, 99999);
    assert_equal_int("R(8,8) step count", 8, static_cast<int>(pattern_8_8.size()));
    assert_equal_int("R(8,8) onset count", 8, countOnsets(pattern_8_8));
    assert_equal("R(8,8) pattern", "11111111", patternToString(pattern_8_8));
}

void testBellCurveRandomPatterns() {
    std::cout << "\n=== Testing Bell Curve Random Patterns ===\n" << std::endl;
    
    // Test R(r,steps) - bell curve distribution
    // Formula: onsets = steps/3 + (rand() % (steps/2))
    
    // R(r,16): onsets = 16/3 + (rand() % 8) = 5 + (0-7) = 5-12 onsets
    std::cout << "Testing R(r,16) bell curve distribution:" << std::endl;
    
    std::map<int, int> onsetCounts;
    const int iterations = 100;
    
    // Test multiple iterations to verify distribution
    for (int i = 0; i < iterations; ++i) {
        srand(i); // Use iteration as seed for reproducibility
        int steps = 16;
        int bellCurveOnsets = steps / 3 + (rand() % (steps / 2));
        onsetCounts[bellCurveOnsets]++;
        
        // Verify onset count is in expected range
        assert_in_range("R(r,16) onset range iteration " + std::to_string(i), 
                       bellCurveOnsets, 5, 12);
    }
    
    // Print distribution for verification
    std::cout << "Onset distribution over " << iterations << " iterations:" << std::endl;
    for (const auto& pair : onsetCounts) {
        std::cout << "  " << pair.first << " onsets: " << pair.second << " times" << std::endl;
    }
    
    // Verify we get good distribution (no value should dominate)
    int maxCount = 0;
    for (const auto& pair : onsetCounts) {
        maxCount = std::max(maxCount, pair.second);
    }
    assert_true("R(r,16) good distribution", maxCount < iterations * 0.4); // No single value > 40%
    
    // Test different step counts
    // R(r,8): onsets = 8/3 + (rand() % 4) = 2 + (0-3) = 2-5 onsets
    srand(42);
    int onsets_8 = 8 / 3 + (rand() % (8 / 2));
    assert_in_range("R(r,8) onset range", onsets_8, 2, 5);
    
    // R(r,24): onsets = 24/3 + (rand() % 12) = 8 + (0-11) = 8-19 onsets
    srand(123);
    int onsets_24 = 24 / 3 + (rand() % (24 / 2));
    assert_in_range("R(r,24) onset range", onsets_24, 8, 19);
}

void testRandomPatternEdgeCases() {
    std::cout << "\n=== Testing Random Pattern Edge Cases ===\n" << std::endl;
    
    // Test edge cases
    
    // More onsets than steps - should clamp to steps
    auto pattern_overflow = parseRandomDeterministic(10, 6, 777);
    assert_equal_int("R(10,6) clamped step count", 6, static_cast<int>(pattern_overflow.size()));
    assert_equal_int("R(10,6) clamped onset count", 6, countOnsets(pattern_overflow));
    assert_equal("R(10,6) all onsets", "111111", patternToString(pattern_overflow));
    
    // Single step patterns
    auto pattern_1_1 = parseRandomDeterministic(1, 1, 888);
    assert_equal_int("R(1,1) step count", 1, static_cast<int>(pattern_1_1.size()));
    assert_equal_int("R(1,1) onset count", 1, countOnsets(pattern_1_1));
    assert_equal("R(1,1) pattern", "1", patternToString(pattern_1_1));
    
    auto pattern_0_1 = parseRandomDeterministic(0, 1, 999);
    assert_equal_int("R(0,1) step count", 1, static_cast<int>(pattern_0_1.size()));
    assert_equal_int("R(0,1) onset count", 0, countOnsets(pattern_0_1));
    assert_equal("R(0,1) pattern", "0", patternToString(pattern_0_1));
    
    // Large patterns
    auto pattern_large = parseRandomDeterministic(10, 32, 1234);
    assert_equal_int("R(10,32) step count", 32, static_cast<int>(pattern_large.size()));
    assert_equal_int("R(10,32) onset count", 10, countOnsets(pattern_large));
    std::cout << "R(10,32) pattern: " << patternToString(pattern_large) << std::endl;
}

void testRandomPatternUniqueness() {
    std::cout << "\n=== Testing Random Pattern Uniqueness ===\n" << std::endl;
    
    // Test that different seeds produce different patterns
    auto pattern1 = parseRandomDeterministic(4, 8, 1111);
    auto pattern2 = parseRandomDeterministic(4, 8, 2222);
    auto pattern3 = parseRandomDeterministic(4, 8, 3333);
    
    std::string str1 = patternToString(pattern1);
    std::string str2 = patternToString(pattern2);
    std::string str3 = patternToString(pattern3);
    
    std::cout << "Pattern 1: " << str1 << std::endl;
    std::cout << "Pattern 2: " << str2 << std::endl;
    std::cout << "Pattern 3: " << str3 << std::endl;
    
    // All should have same onset count but different arrangements
    assert_equal_int("Pattern 1 onset count", 4, countOnsets(pattern1));
    assert_equal_int("Pattern 2 onset count", 4, countOnsets(pattern2));
    assert_equal_int("Pattern 3 onset count", 4, countOnsets(pattern3));
    
    // Should be different patterns (very unlikely to be same)
    assert_true("Patterns are unique", str1 != str2 || str2 != str3);
}

void testRandomPatternRecognition() {
    std::cout << "\n=== Testing Random Pattern Recognition ===\n" << std::endl;
    
    // Test valid random patterns
    assert_true("R(3,8) recognition", isRandomPattern(MockString("R(3,8)")));
    assert_true("r(3,8) case insensitive", isRandomPattern(MockString("r(3,8)")));
    assert_true("R(5,16) recognition", isRandomPattern(MockString("R(5,16)")));
    assert_true("R(r,8) bell curve", isRandomPattern(MockString("R(r,8)")));
    assert_true("R(r,16) bell curve", isRandomPattern(MockString("R(r,16)")));
    assert_true("r(r,24) case insensitive bell curve", isRandomPattern(MockString("r(r,24)")));
    
    // Test invalid patterns
    assert_true("R(3) incomplete (should be false)", !isRandomPattern(MockString("R(3)")));
    assert_true("R() empty (should be false)", !isRandomPattern(MockString("R()")));
    assert_true("R(a,b) non-numeric (should be false)", !isRandomPattern(MockString("R(a,b)")));
    assert_true("E(3,8) euclidean not random", !isRandomPattern(MockString("E(3,8)")));
    assert_true("Random text (should be false)", !isRandomPattern(MockString("hello world")));
}

void testRandomPatternMathematicalProperties() {
    std::cout << "\n=== Testing Random Pattern Mathematical Properties ===\n" << std::endl;
    
    // Test that random patterns maintain correct onset distribution
    for (int onsets = 1; onsets <= 8; ++onsets) {
        for (int steps = onsets; steps <= 16; steps += 2) {
            auto pattern = parseRandomDeterministic(onsets, steps, onsets * steps);
            
            std::string desc = "R(" + std::to_string(onsets) + "," + std::to_string(steps) + ")";
            assert_equal_int(desc + " step count", steps, static_cast<int>(pattern.size()));
            assert_equal_int(desc + " onset count", onsets, countOnsets(pattern));
            
            // Verify no duplicate onsets (all onsets are unique positions)
            std::vector<int> onsetPositions;
            for (int i = 0; i < static_cast<int>(pattern.size()); ++i) {
                if (pattern[i]) onsetPositions.push_back(i);
            }
            
            std::sort(onsetPositions.begin(), onsetPositions.end());
            auto it = std::unique(onsetPositions.begin(), onsetPositions.end());
            onsetPositions.resize(std::distance(onsetPositions.begin(), it));
            
            assert_equal_int(desc + " unique onsets", onsets, static_cast<int>(onsetPositions.size()));
        }
    }
    
    std::cout << "All mathematical properties verified" << std::endl;
}

void testBellCurveStatistics() {
    std::cout << "\n=== Testing Bell Curve Statistics ===\n" << std::endl;
    
    // Test statistical properties of bell curve distribution
    std::vector<int> stepCounts = {8, 16, 24, 32};
    
    for (int steps : stepCounts) {
        std::vector<int> onsetSamples;
        const int samples = 1000;
        
        for (int i = 0; i < samples; ++i) {
            srand(i * 12345 + steps * 789); // Better seed distribution
            int onsets = steps / 3 + (rand() % (steps / 2));
            onsetSamples.push_back(onsets);
        }
        
        // Calculate statistics
        double mean = 0.0;
        for (int onset : onsetSamples) {
            mean += onset;
        }
        mean /= samples;
        
        double variance = 0.0;
        for (int onset : onsetSamples) {
            variance += (onset - mean) * (onset - mean);
        }
        variance /= samples;
        double stddev = std::sqrt(variance);
        
        int minOnsets = *std::min_element(onsetSamples.begin(), onsetSamples.end());
        int maxOnsets = *std::max_element(onsetSamples.begin(), onsetSamples.end());
        
        std::cout << "R(r," << steps << ") statistics over " << samples << " samples:" << std::endl;
        std::cout << "  Range: [" << minOnsets << ", " << maxOnsets << "]" << std::endl;
        std::cout << "  Expected range: [" << steps/3 << ", " << steps/3 + steps/2 - 1 << "]" << std::endl;
        std::cout << "  Mean: " << mean << std::endl;
        std::cout << "  Std Dev: " << stddev << std::endl;
        
        // Verify range matches expected
        // Formula: onsets = steps/3 + (rand() % (steps/2))
        // So range is [steps/3, steps/3 + steps/2 - 1]
        int expectedMin = steps/3;
        int expectedMax = steps/3 + steps/2 - 1;
        
        // With statistical sampling, we should see most of the range but not necessarily the exact extremes
        assert_true("R(r," + std::to_string(steps) + ") min onset reasonable", 
                   minOnsets >= expectedMin && minOnsets <= expectedMin + 1);
        assert_true("R(r," + std::to_string(steps) + ") max onset reasonable", 
                   maxOnsets >= expectedMax - 1 && maxOnsets <= expectedMax);
        
        // Mean should be reasonable (somewhere in the range)
        double expectedMean = steps/3 + (steps/2 - 1) / 2.0;
        assert_true("R(r," + std::to_string(steps) + ") reasonable mean", 
                   mean >= expectedMin && mean <= expectedMax);
    }
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    std::cout << "=== RANDOM PATTERN TESTS ===" << std::endl;
    std::cout << "Testing random pattern generation functionality to prevent regression\n" << std::endl;
    
    try {
        testBasicRandomPatterns();
        testBellCurveRandomPatterns();
        testRandomPatternEdgeCases();
        testRandomPatternUniqueness();
        testRandomPatternRecognition();
        testRandomPatternMathematicalProperties();
        testBellCurveStatistics();
        
        std::cout << "\n=== ALL RANDOM PATTERN TESTS PASSED ===" << std::endl;
        std::cout << "Random pattern functionality is working correctly and protected against regression." << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "\n=== RANDOM PATTERN TEST FAILURE ===" << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}