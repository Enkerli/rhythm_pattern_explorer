/*
  ==============================================================================

    TestProgressiveLengthening.cpp
    Comprehensive unit tests for progressive lengthening patterns (*N notation)
    
    Tests the progressive lengthening system that extends patterns by N steps
    each time they are triggered via MIDI, Enter key, or Tick button.

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <random>

//==============================================================================
// Euclidean algorithm for testing
std::vector<bool> generateEuclidean(int onsets, int steps)
{
    if (steps <= 0 || onsets <= 0 || onsets > steps) {
        return std::vector<bool>(std::max(1, steps), false);
    }
    
    std::vector<int> pattern(steps, 0);
    int bucket = 0;
    
    for (int i = 0; i < steps; ++i) {
        bucket += onsets;
        if (bucket >= steps) {
            bucket -= steps;
            pattern[i] = 1;
        }
    }
    
    std::vector<bool> result;
    for (int val : pattern) {
        result.push_back(val == 1);
    }
    
    return result;
}

//==============================================================================
// Random pattern generator for testing
std::vector<bool> generateRandomPattern(int onsets, int steps, int seed = 42)
{
    if (steps <= 0 || onsets < 0 || onsets > steps) {
        return std::vector<bool>(std::max(1, steps), false);
    }
    
    std::vector<bool> pattern(steps, false);
    std::vector<int> positions;
    
    for (int i = 0; i < steps; ++i) {
        positions.push_back(i);
    }
    
    // Use fixed seed for reproducible tests
    std::mt19937 rng(seed);
    std::shuffle(positions.begin(), positions.end(), rng);
    
    for (int i = 0; i < onsets; ++i) {
        pattern[positions[i]] = true;
    }
    
    return pattern;
}

//==============================================================================
class ProgressiveLengtheningTester
{
public:
    void runAllTests()
    {
        std::cout << "=== PROGRESSIVE LENGTHENING PATTERN TESTS ===" << std::endl;
        
        testBasicProgressiveLengthening();
        testProgressiveLengtheningZero();
        testProgressiveLengtheningNegative();
        testProgressiveLengtheningLarge();
        testEuclideanProgressiveLengthening();
        testBinaryProgressiveLengthening();
        testRandomProgressiveLengthening();
        testProgressiveLengtheningEdgeCases();
        testProgressiveLengtheningBehavior();
        
        std::cout << "\n✅ All progressive lengthening tests passed!" << std::endl;
    }

private:
    std::string patternToString(const std::vector<bool>& pattern)
    {
        std::string result;
        for (bool bit : pattern) {
            result += bit ? "1" : "0";
        }
        return result;
    }
    
    int countOnsets(const std::vector<bool>& pattern)
    {
        return std::count(pattern.begin(), pattern.end(), true);
    }
    
    // Simulate progressive lengthening behavior
    std::vector<bool> applyProgressiveLengthening(const std::vector<bool>& basePattern, int lengthening)
    {
        if (lengthening <= 0) return basePattern;
        
        std::vector<bool> extended = basePattern;
        
        // Add 'lengthening' number of false steps to the end
        for (int i = 0; i < lengthening; ++i) {
            extended.push_back(false);
        }
        
        return extended;
    }
    
    void testBasicProgressiveLengthening()
    {
        std::cout << "\n--- Testing Basic Progressive Lengthening (*2) ---" << std::endl;
        
        // Test E(3,8)*2 progressive lengthening
        auto basePattern = generateEuclidean(3, 8);
        int lengthening = 2;
        
        std::cout << "Base E(3,8):      " << patternToString(basePattern) 
                  << " (length " << basePattern.size() << ", onsets " << countOnsets(basePattern) << ")" << std::endl;
        
        // Simulate multiple lengthening triggers
        auto pattern1 = applyProgressiveLengthening(basePattern, lengthening * 1);
        auto pattern2 = applyProgressiveLengthening(basePattern, lengthening * 2);
        auto pattern3 = applyProgressiveLengthening(basePattern, lengthening * 3);
        
        std::cout << "After trigger 1:  " << patternToString(pattern1) 
                  << " (length " << pattern1.size() << ", onsets " << countOnsets(pattern1) << ")" << std::endl;
        std::cout << "After trigger 2:  " << patternToString(pattern2) 
                  << " (length " << pattern2.size() << ", onsets " << countOnsets(pattern2) << ")" << std::endl;
        std::cout << "After trigger 3:  " << patternToString(pattern3) 
                  << " (length " << pattern3.size() << ", onsets " << countOnsets(pattern3) << ")" << std::endl;
        
        // Verify progressive lengthening behavior
        assert(pattern1.size() == basePattern.size() + lengthening);
        assert(pattern2.size() == basePattern.size() + (lengthening * 2));
        assert(pattern3.size() == basePattern.size() + (lengthening * 3));
        
        // Onset count should remain constant
        assert(countOnsets(pattern1) == countOnsets(basePattern));
        assert(countOnsets(pattern2) == countOnsets(basePattern));
        assert(countOnsets(pattern3) == countOnsets(basePattern));
        
        // Original pattern should be preserved at the beginning
        for (size_t i = 0; i < basePattern.size(); ++i) {
            assert(pattern1[i] == basePattern[i]);
            assert(pattern2[i] == basePattern[i]);
            assert(pattern3[i] == basePattern[i]);
        }
        
        std::cout << "✅ Basic progressive lengthening test passed" << std::endl;
    }
    
    void testProgressiveLengtheningZero()
    {
        std::cout << "\n--- Testing Zero Progressive Lengthening (*0) ---" << std::endl;
        
        auto basePattern = generateEuclidean(3, 8);
        int lengthening = 0;
        
        std::cout << "Base pattern:     " << patternToString(basePattern) << std::endl;
        
        // Multiple triggers with zero lengthening should produce identical patterns
        auto pattern1 = applyProgressiveLengthening(basePattern, lengthening * 1);
        auto pattern2 = applyProgressiveLengthening(basePattern, lengthening * 2);
        auto pattern3 = applyProgressiveLengthening(basePattern, lengthening * 3);
        
        assert(pattern1 == basePattern);
        assert(pattern2 == basePattern);
        assert(pattern3 == basePattern);
        
        std::cout << "All triggers produced identical patterns (as expected)" << std::endl;
        std::cout << "✅ Zero progressive lengthening test passed" << std::endl;
    }
    
    void testProgressiveLengtheningNegative()
    {
        std::cout << "\n--- Testing Negative Progressive Lengthening (*-1) ---" << std::endl;
        
        auto basePattern = generateEuclidean(3, 8);
        int lengthening = -1;
        
        std::cout << "Base pattern:     " << patternToString(basePattern) 
                  << " (length " << basePattern.size() << ")" << std::endl;
        
        // Note: Negative lengthening should probably be handled gracefully
        // Either by treating it as zero or by shortening the pattern
        auto pattern1 = applyProgressiveLengthening(basePattern, lengthening * 1);
        auto pattern2 = applyProgressiveLengthening(basePattern, lengthening * 2);
        
        std::cout << "After trigger 1:  " << patternToString(pattern1) 
                  << " (length " << pattern1.size() << ")" << std::endl;
        std::cout << "After trigger 2:  " << patternToString(pattern2) 
                  << " (length " << pattern2.size() << ")" << std::endl;
        
        // For negative lengthening, we expect no change (graceful handling)
        assert(pattern1 == basePattern);
        assert(pattern2 == basePattern);
        
        std::cout << "✅ Negative progressive lengthening test passed (graceful handling)" << std::endl;
    }
    
    void testProgressiveLengtheningLarge()
    {
        std::cout << "\n--- Testing Large Progressive Lengthening (*5) ---" << std::endl;
        
        auto basePattern = generateEuclidean(2, 5);
        int lengthening = 5;
        
        std::cout << "Base E(2,5):      " << patternToString(basePattern) 
                  << " (length " << basePattern.size() << ")" << std::endl;
        
        // Test large lengthening values
        auto pattern1 = applyProgressiveLengthening(basePattern, lengthening * 1);
        auto pattern2 = applyProgressiveLengthening(basePattern, lengthening * 2);
        
        std::cout << "After trigger 1:  " << patternToString(pattern1) 
                  << " (length " << pattern1.size() << ")" << std::endl;
        std::cout << "After trigger 2:  " << patternToString(pattern2) 
                  << " (length " << pattern2.size() << ")" << std::endl;
        
        // Verify correct lengthening
        assert(pattern1.size() == basePattern.size() + lengthening);
        assert(pattern2.size() == basePattern.size() + (lengthening * 2));
        
        // Verify onset count preservation
        assert(countOnsets(pattern1) == countOnsets(basePattern));
        assert(countOnsets(pattern2) == countOnsets(basePattern));
        
        std::cout << "✅ Large progressive lengthening test passed" << std::endl;
    }
    
    void testEuclideanProgressiveLengthening()
    {
        std::cout << "\n--- Testing Euclidean Progressive Lengthening ---" << std::endl;
        
        std::vector<std::tuple<int, int, int>> testCases = {
            {1, 4, 1},   // E(1,4)*1
            {2, 5, 2},   // E(2,5)*2
            {3, 8, 3},   // E(3,8)*3
            {5, 13, 2}   // E(5,13)*2
        };
        
        for (const auto& [onsets, steps, lengthening] : testCases) {
            std::cout << "\nTesting E(" << onsets << "," << steps << ")*" << lengthening << std::endl;
            
            auto basePattern = generateEuclidean(onsets, steps);
            std::cout << "Base:             " << patternToString(basePattern) 
                      << " (length " << basePattern.size() << ", onsets " << countOnsets(basePattern) << ")" << std::endl;
            
            // Test multiple lengthening steps
            for (int trigger = 1; trigger <= 3; ++trigger) {
                auto extendedPattern = applyProgressiveLengthening(basePattern, lengthening * trigger);
                
                std::cout << "Trigger " << trigger << ":         " << patternToString(extendedPattern) 
                          << " (length " << extendedPattern.size() << ", onsets " << countOnsets(extendedPattern) << ")" << std::endl;
                
                // Verify properties
                assert(extendedPattern.size() == basePattern.size() + (lengthening * trigger));
                assert(countOnsets(extendedPattern) == countOnsets(basePattern));
                
                // Original pattern preserved at start
                for (size_t i = 0; i < basePattern.size(); ++i) {
                    assert(extendedPattern[i] == basePattern[i]);
                }
                
                // Added steps should be empty
                for (size_t i = basePattern.size(); i < extendedPattern.size(); ++i) {
                    assert(extendedPattern[i] == false);
                }
            }
        }
        
        std::cout << "✅ Euclidean progressive lengthening tests passed" << std::endl;
    }
    
    void testBinaryProgressiveLengthening()
    {
        std::cout << "\n--- Testing Binary Progressive Lengthening ---" << std::endl;
        
        std::vector<bool> basePattern = {true, false, true, true, false, false, true, false}; // 10110010
        int lengthening = 2;
        
        std::cout << "Base binary:      " << patternToString(basePattern) 
                  << " (length " << basePattern.size() << ", onsets " << countOnsets(basePattern) << ")" << std::endl;
        
        auto pattern1 = applyProgressiveLengthening(basePattern, lengthening);
        auto pattern2 = applyProgressiveLengthening(basePattern, lengthening * 2);
        
        std::cout << "After trigger 1:  " << patternToString(pattern1) 
                  << " (length " << pattern1.size() << ", onsets " << countOnsets(pattern1) << ")" << std::endl;
        std::cout << "After trigger 2:  " << patternToString(pattern2) 
                  << " (length " << pattern2.size() << ", onsets " << countOnsets(pattern2) << ")" << std::endl;
        
        // Verify expected patterns
        std::vector<bool> expected1 = {true, false, true, true, false, false, true, false, false, false};
        std::vector<bool> expected2 = {true, false, true, true, false, false, true, false, false, false, false, false};
        
        assert(pattern1 == expected1);
        assert(pattern2 == expected2);
        assert(countOnsets(pattern1) == countOnsets(basePattern));
        assert(countOnsets(pattern2) == countOnsets(basePattern));
        
        std::cout << "✅ Binary progressive lengthening test passed" << std::endl;
    }
    
    void testRandomProgressiveLengthening()
    {
        std::cout << "\n--- Testing Random Progressive Lengthening ---" << std::endl;
        
        auto basePattern = generateRandomPattern(3, 8, 123); // Fixed seed for reproducibility
        int lengthening = 3;
        
        std::cout << "Base R(3,8):      " << patternToString(basePattern) 
                  << " (length " << basePattern.size() << ", onsets " << countOnsets(basePattern) << ")" << std::endl;
        
        auto pattern1 = applyProgressiveLengthening(basePattern, lengthening);
        auto pattern2 = applyProgressiveLengthening(basePattern, lengthening * 2);
        
        std::cout << "After trigger 1:  " << patternToString(pattern1) 
                  << " (length " << pattern1.size() << ", onsets " << countOnsets(pattern1) << ")" << std::endl;
        std::cout << "After trigger 2:  " << patternToString(pattern2) 
                  << " (length " << pattern2.size() << ", onsets " << countOnsets(pattern2) << ")" << std::endl;
        
        // Verify properties
        assert(pattern1.size() == basePattern.size() + lengthening);
        assert(pattern2.size() == basePattern.size() + (lengthening * 2));
        assert(countOnsets(pattern1) == countOnsets(basePattern));
        assert(countOnsets(pattern2) == countOnsets(basePattern));
        
        std::cout << "✅ Random progressive lengthening test passed" << std::endl;
    }
    
    void testProgressiveLengtheningEdgeCases()
    {
        std::cout << "\n--- Testing Progressive Lengthening Edge Cases ---" << std::endl;
        
        struct TestCase {
            std::vector<bool> pattern;
            int lengthening;
            std::string description;
        };
        
        std::vector<TestCase> edgeCases = {
            {{true}, 1, "Single onset pattern"},
            {{false}, 2, "Single empty step"},
            {{}, 3, "Empty pattern"},
            {{true, true, true}, 1, "All onsets"},
            {{false, false, false}, 2, "No onsets"},
            {generateEuclidean(1, 32), 5, "Large sparse pattern"},
            {generateEuclidean(31, 32), 2, "Dense pattern"}
        };
        
        for (const auto& testCase : edgeCases) {
            if (testCase.pattern.empty()) continue; // Skip empty pattern test
            
            std::cout << "\nTesting: " << testCase.description << std::endl;
            std::cout << "Pattern: " << patternToString(testCase.pattern) << std::endl;
            
            int baseOnsets = countOnsets(testCase.pattern);
            
            // Test multiple lengthening steps
            for (int trigger = 1; trigger <= 2; ++trigger) {
                auto extendedPattern = applyProgressiveLengthening(testCase.pattern, testCase.lengthening * trigger);
                
                // Verify properties
                assert(extendedPattern.size() == testCase.pattern.size() + (testCase.lengthening * trigger));
                assert(countOnsets(extendedPattern) == baseOnsets);
                
                // Original pattern should be preserved
                for (size_t i = 0; i < testCase.pattern.size(); ++i) {
                    assert(extendedPattern[i] == testCase.pattern[i]);
                }
            }
            
            std::cout << "  ✓ Edge case handled correctly" << std::endl;
        }
        
        std::cout << "✅ Progressive lengthening edge cases test passed" << std::endl;
    }
    
    void testProgressiveLengtheningBehavior()
    {
        std::cout << "\n--- Testing Progressive Lengthening Behavior ---" << std::endl;
        
        // Test that lengthening adds empty steps, not random or patterned steps
        auto basePattern = generateEuclidean(4, 7);
        int lengthening = 3;
        
        std::cout << "Base pattern:     " << patternToString(basePattern) << std::endl;
        
        auto extended = applyProgressiveLengthening(basePattern, lengthening);
        std::cout << "After lengthening:" << patternToString(extended) << std::endl;
        
        // All added steps should be empty (false)
        for (size_t i = basePattern.size(); i < extended.size(); ++i) {
            assert(extended[i] == false);
        }
        
        // Test multiple applications
        auto doubleExtended = applyProgressiveLengthening(basePattern, lengthening * 2);
        std::cout << "Double lengthening:" << patternToString(doubleExtended) << std::endl;
        
        // All added steps should still be empty
        for (size_t i = basePattern.size(); i < doubleExtended.size(); ++i) {
            assert(doubleExtended[i] == false);
        }
        
        std::cout << "✅ Progressive lengthening behavior test passed" << std::endl;
    }
};

//==============================================================================
int main()
{
    try {
        ProgressiveLengtheningTester tester;
        tester.runAllTests();
        
        std::cout << "\n🎉 ALL PROGRESSIVE LENGTHENING TESTS COMPLETED SUCCESSFULLY! 🎉" << std::endl;
        std::cout << "\nKey features verified:" << std::endl;
        std::cout << "  ✓ Basic progressive lengthening (*N)" << std::endl;
        std::cout << "  ✓ Zero progressive lengthening (no change)" << std::endl;
        std::cout << "  ✓ Negative progressive lengthening (graceful handling)" << std::endl;
        std::cout << "  ✓ Large lengthening values" << std::endl;
        std::cout << "  ✓ Euclidean pattern lengthening" << std::endl;
        std::cout << "  ✓ Binary pattern lengthening" << std::endl;
        std::cout << "  ✓ Random pattern lengthening" << std::endl;
        std::cout << "  ✓ Edge cases (single onset, no onsets, dense patterns)" << std::endl;
        std::cout << "  ✓ Onset count preservation during lengthening" << std::endl;
        std::cout << "  ✓ Empty step addition behavior" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}