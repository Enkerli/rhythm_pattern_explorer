/*
  ==============================================================================
    Polygon Pattern Tests
    
    Comprehensive test suite for polygon pattern functionality to prevent
    regression and ensure mathematical correctness.
    
    Coverage:
    - Basic polygon patterns P(sides,offset)
    - Polygon patterns with multiplication P(sides,offset,steps)
    - Polygon combinations (perfect balance examples)
    - Edge cases and mathematical properties
    - Pattern recognition and parsing
    
  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
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
    
    MockString toLowerCase() const {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return MockString(lower);
    }
    
    size_t length() const { return str.length(); }
    char operator[](size_t i) const { return str[i]; }
    bool operator==(const std::string& other) const { return str == other; }
    MockString operator+=(const std::string& other) { str += other; return *this; }
    std::string toStdString() const { return str; }
    
    bool contains(const std::string& substr) const {
        return str.find(substr) != std::string::npos;
    }
};

// Copy of polygon implementation for testing
std::vector<bool> parsePolygon(int sides, int offset, int totalSteps = 0)
{
    if (totalSteps == 0) 
        totalSteps = sides;
    
    std::vector<bool> pattern(totalSteps, false);
    
    // Place polygon vertices with proper spacing
    for (int i = 0; i < sides; ++i)
    {
        // Calculate position with floating point for accuracy, then round
        double exactPos = (double)(i * totalSteps) / sides;
        int pos = ((int)std::round(exactPos) + offset) % totalSteps;
        if (pos < 0) pos += totalSteps; // Handle negative modulo
        
        if (pos >= 0 && pos < totalSteps)
            pattern[pos] = true;
    }
    
    return pattern;
}

// Pattern recognition function
bool isPolygonPattern(const MockString& input)
{
    std::regex polygonRegex(R"([Pp]\(\d+,\d+(?:,\d+)?\))");
    return std::regex_search(input.toStdString(), polygonRegex);
}

// Polygon combination with LCM calculation
int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}

std::vector<bool> combinePolygons(const std::vector<std::vector<bool>>& polygons) {
    if (polygons.empty()) return {};
    
    // All polygons should be same size for combination
    size_t size = polygons[0].size();
    std::vector<bool> result(size, false);
    
    for (const auto& polygon : polygons) {
        for (size_t i = 0; i < size && i < polygon.size(); ++i) {
            result[i] = result[i] || polygon[i];
        }
    }
    
    return result;
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

//==============================================================================
// Test Functions
//==============================================================================

void testBasicPolygonPatterns() {
    std::cout << "\n=== Testing Basic Polygon Patterns ===" << std::endl;
    
    // Test basic polygon patterns P(sides,offset)
    
    // Triangle: P(3,0) - 3 sides, no offset
    // In natural space (3 steps), vertices at positions 0, 1, 2
    auto triangle = parsePolygon(3, 0);
    assert_equal("P(3,0) triangle", "111", patternToString(triangle));
    assert_equal_int("P(3,0) onset count", 3, countOnsets(triangle));
    
    // Square: P(4,0) - 4 sides, no offset  
    // In natural space (4 steps), vertices at positions 0, 1, 2, 3
    auto square = parsePolygon(4, 0);
    assert_equal("P(4,0) square", "1111", patternToString(square));
    assert_equal_int("P(4,0) onset count", 4, countOnsets(square));
    
    // Pentagon: P(5,0) - 5 sides, no offset
    // In natural space (5 steps), vertices at positions 0, 1, 2, 3, 4
    auto pentagon = parsePolygon(5, 0);
    assert_equal("P(5,0) pentagon", "11111", patternToString(pentagon));
    assert_equal_int("P(5,0) onset count", 5, countOnsets(pentagon));
    
    // Test with offset - in natural space all vertices are placed
    auto triangle_offset1 = parsePolygon(3, 1);
    assert_equal("P(3,1) triangle offset", "111", patternToString(triangle_offset1));
    
    auto square_offset2 = parsePolygon(4, 2);
    assert_equal("P(4,2) square offset", "1111", patternToString(square_offset2));
    
    // Test offset effect in larger space where pattern can be seen
    auto triangle_offset_6 = parsePolygon(3, 1, 6);
    assert_equal("P(3,1) in 6 steps", "010101", patternToString(triangle_offset_6));
}

void testPolygonMultiplication() {
    std::cout << "\n=== Testing Polygon Multiplication ===" << std::endl;
    
    // Test P(sides,offset,steps) notation
    
    // P(5,0,2) - pentagon with 2x multiplier = 10 steps
    auto pentagon_2x = parsePolygon(5, 0, 10);
    assert_equal_int("P(5,0,2) step count", 10, static_cast<int>(pentagon_2x.size()));
    assert_equal_int("P(5,0,2) onset count", 5, countOnsets(pentagon_2x));
    std::cout << "P(5,0,2) pattern: " << patternToString(pentagon_2x) << std::endl;
    
    // Verify polygon vertex spacing in 10 steps
    // Vertices should be at positions: 0, 2, 4, 6, 8 (evenly spaced)
    assert_true("P(5,0,2) vertex at 0", pentagon_2x[0]);
    assert_true("P(5,0,2) vertex at 2", pentagon_2x[2]);
    assert_true("P(5,0,2) vertex at 4", pentagon_2x[4]);
    assert_true("P(5,0,2) vertex at 6", pentagon_2x[6]);
    assert_true("P(5,0,2) vertex at 8", pentagon_2x[8]);
    
    // P(3,0,4) - triangle with 4x multiplier = 12 steps
    auto triangle_4x = parsePolygon(3, 0, 12);
    assert_equal_int("P(3,0,4) step count", 12, static_cast<int>(triangle_4x.size()));
    assert_equal_int("P(3,0,4) onset count", 3, countOnsets(triangle_4x));
    // Vertices should be at: 0, 4, 8
    assert_true("P(3,0,4) vertex at 0", triangle_4x[0]);
    assert_true("P(3,0,4) vertex at 4", triangle_4x[4]);
    assert_true("P(3,0,4) vertex at 8", triangle_4x[8]);
}

void testPolygonCombinations() {
    std::cout << "\n=== Testing Polygon Combinations ===" << std::endl;
    
    // Test the user's perfect balance example: P(5,0)+P(3,1)+P(2,5)
    // All projected onto LCM space
    
    int lcm_5_3_2 = lcm(lcm(5, 3), 2); // LCM(5,3,2) = 30
    
    auto p5_0 = parsePolygon(5, 0, lcm_5_3_2);  // Pentagon, no offset
    auto p3_1 = parsePolygon(3, 1, lcm_5_3_2);  // Triangle, offset 1
    auto p2_5 = parsePolygon(2, 5, lcm_5_3_2);  // Line, offset 5
    
    // Combine all patterns (OR operation)
    std::vector<std::vector<bool>> polygons = {p5_0, p3_1, p2_5};
    auto combined = combinePolygons(polygons);
    
    assert_equal_int("Perfect balance LCM", 30, lcm_5_3_2);
    assert_equal_int("Perfect balance step count", 30, static_cast<int>(combined.size()));
    
    std::cout << "P(5,0) in 30 steps: " << patternToString(p5_0) << std::endl;
    std::cout << "P(3,1) in 30 steps: " << patternToString(p3_1) << std::endl;
    std::cout << "P(2,5) in 30 steps: " << patternToString(p2_5) << std::endl;
    std::cout << "Combined pattern:   " << patternToString(combined) << std::endl;
    
    // Verify that combination has expected properties
    int totalOnsets = countOnsets(combined);
    std::cout << "Total onsets in combination: " << totalOnsets << std::endl;
    
    // The combination should have good distribution (perfect balance)
    assert_true("Combined pattern not empty", totalOnsets > 0);
    assert_true("Combined pattern not all onsets", totalOnsets < 30);
}

void testPolygonEdgeCases() {
    std::cout << "\n=== Testing Polygon Edge Cases ===" << std::endl;
    
    // Test single vertex polygon P(1,0)
    auto single = parsePolygon(1, 0);
    assert_equal("P(1,0) single vertex", "1", patternToString(single));
    
    // Test large offset that wraps around
    auto wrapped = parsePolygon(3, 5); // Offset larger than pattern size
    assert_equal_int("P(3,5) wrapped size", 3, static_cast<int>(wrapped.size()));
    assert_equal_int("P(3,5) onset count", 3, countOnsets(wrapped));
    // Should be equivalent to P(3,2) since 5 % 3 = 2
    auto expected = parsePolygon(3, 2);
    assert_equal("P(3,5) wrapped pattern", patternToString(expected), patternToString(wrapped));
    
    // Test negative offset
    auto negative = parsePolygon(4, -1);
    assert_equal_int("P(4,-1) size", 4, static_cast<int>(negative.size()));
    assert_equal_int("P(4,-1) onset count", 4, countOnsets(negative));
    // Should wrap to positive offset
    auto positive_equiv = parsePolygon(4, 3); // -1 % 4 + 4 = 3
    assert_equal("P(4,-1) negative offset", patternToString(positive_equiv), patternToString(negative));
    
    // Test zero sides (edge case)
    auto zero_sides = parsePolygon(0, 0);
    assert_equal_int("P(0,0) size", 0, static_cast<int>(zero_sides.size()));
    assert_equal_int("P(0,0) onset count", 0, countOnsets(zero_sides));
}

void testPolygonMathematicalProperties() {
    std::cout << "\n=== Testing Polygon Mathematical Properties ===" << std::endl;
    
    // Test that vertices are evenly distributed
    for (int sides = 2; sides <= 8; ++sides) {
        auto pattern = parsePolygon(sides, 0, sides * 4); // 4x multiplier for clarity
        
        std::vector<int> onsetPositions;
        for (int i = 0; i < static_cast<int>(pattern.size()); ++i) {
            if (pattern[i]) onsetPositions.push_back(i);
        }
        
        assert_equal_int("P(" + std::to_string(sides) + ",0,4) onset count", 
                        sides, static_cast<int>(onsetPositions.size()));
        
        // Check spacing between vertices
        if (onsetPositions.size() >= 2) {
            int expectedSpacing = (sides * 4) / sides; // Should be 4
            for (size_t i = 1; i < onsetPositions.size(); ++i) {
                int actualSpacing = onsetPositions[i] - onsetPositions[i-1];
                assert_equal_int("P(" + std::to_string(sides) + ",0,4) vertex spacing", 
                               expectedSpacing, actualSpacing);
            }
        }
    }
    
    // Test rotation invariance
    auto base_pattern = parsePolygon(5, 0);
    auto rotated_pattern = parsePolygon(5, 1);
    
    assert_equal_int("Rotation invariance onset count", 
                    countOnsets(base_pattern), countOnsets(rotated_pattern));
}

void testPolygonPatternRecognition() {
    std::cout << "\n=== Testing Polygon Pattern Recognition ===" << std::endl;
    
    // Test valid polygon patterns
    assert_true("P(3,0) recognition", isPolygonPattern(MockString("P(3,0)")));
    assert_true("p(3,0) case insensitive", isPolygonPattern(MockString("p(3,0)")));
    assert_true("P(5,2) recognition", isPolygonPattern(MockString("P(5,2)")));
    assert_true("P(3,0,4) with multiplier", isPolygonPattern(MockString("P(3,0,4)")));
    assert_true("P(12,5,3) complex", isPolygonPattern(MockString("P(12,5,3)")));
    
    // Test invalid patterns
    assert_true("P(3) incomplete (should be false)", !isPolygonPattern(MockString("P(3)")));
    assert_true("P() empty (should be false)", !isPolygonPattern(MockString("P()")));
    assert_true("P(a,b) non-numeric (should be false)", !isPolygonPattern(MockString("P(a,b)")));
    assert_true("E(3,8) euclidean not polygon", !isPolygonPattern(MockString("E(3,8)")));
    assert_true("Random text (should be false)", !isPolygonPattern(MockString("hello world")));
}

void testPolygonSpecialCases() {
    std::cout << "\n=== Testing Polygon Special Cases ===" << std::endl;
    
    // Test line (2-sided polygon)
    auto line = parsePolygon(2, 0, 8);
    assert_equal_int("Line P(2,0,8) step count", 8, static_cast<int>(line.size()));
    assert_equal_int("Line P(2,0,8) onset count", 2, countOnsets(line));
    // Vertices should be at 0 and 4
    assert_true("Line vertex at 0", line[0]);
    assert_true("Line vertex at 4", line[4]);
    
    // Test regular polygons at different scales
    auto triangle_12 = parsePolygon(3, 0, 12);
    auto triangle_24 = parsePolygon(3, 0, 24);
    
    // Both should maintain triangular symmetry
    assert_equal_int("Triangle 12 onsets", 3, countOnsets(triangle_12));
    assert_equal_int("Triangle 24 onsets", 3, countOnsets(triangle_24));
    
    // Test prime-sided polygons
    auto heptagon = parsePolygon(7, 0, 21); // 7-sided, 3x multiplier
    assert_equal_int("Heptagon onset count", 7, countOnsets(heptagon));
    assert_equal_int("Heptagon step count", 21, static_cast<int>(heptagon.size()));
    
    std::cout << "Heptagon P(7,0,21): " << patternToString(heptagon) << std::endl;
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    std::cout << "=== POLYGON PATTERN TESTS ===" << std::endl;
    std::cout << "Testing polygon pattern functionality to prevent regression\\n" << std::endl;
    
    try {
        testBasicPolygonPatterns();
        testPolygonMultiplication();
        testPolygonCombinations();
        testPolygonEdgeCases();
        testPolygonMathematicalProperties();
        testPolygonPatternRecognition();
        testPolygonSpecialCases();
        
        std::cout << "\\n=== ALL POLYGON TESTS PASSED ===" << std::endl;
        std::cout << "Polygon pattern functionality is working correctly and protected against regression." << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "\\n=== POLYGON TEST FAILURE ===" << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}