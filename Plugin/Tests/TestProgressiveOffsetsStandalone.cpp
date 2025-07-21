/*
  ==============================================================================

    TestProgressiveOffsetsStandalone.cpp
    Comprehensive unit tests for progressive offset patterns (+N notation)
    
    Standalone version that tests the core progressive offset logic without JUCE dependencies.

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

//==============================================================================
// Simplified PatternEngine for testing progressive offsets
class TestPatternEngine
{
public:
    TestPatternEngine() = default;
    
    void setPattern(const std::vector<bool>& pattern)
    {
        basePattern = pattern;
        currentPattern = pattern;
    }
    
    void setProgressiveOffset(bool enabled, int initial = 0, int progressive = 0)
    {
        hasProgressiveOffset = enabled;
        initialOffset = initial;
        progressiveOffset = progressive;
        currentOffset = initial;
        triggerCount = 0;
    }
    
    void triggerProgressiveOffset()
    {
        if (!hasProgressiveOffset) return;
        
        triggerCount++;
        currentOffset = initialOffset + (triggerCount * progressiveOffset);
        
        // Apply rotation to base pattern
        currentPattern = rotatePattern(basePattern, currentOffset);
    }
    
    const std::vector<bool>& getCurrentPattern() const { return currentPattern; }
    int getCurrentOffset() const { return currentOffset; }
    int getProgressiveTriggerCount() const { return triggerCount; }
    bool hasProgressiveOffsetEnabled() const { return hasProgressiveOffset; }
    
private:
    std::vector<bool> basePattern;
    std::vector<bool> currentPattern;
    bool hasProgressiveOffset = false;
    int initialOffset = 0;
    int progressiveOffset = 0;
    int currentOffset = 0;
    int triggerCount = 0;
    
    std::vector<bool> rotatePattern(const std::vector<bool>& pattern, int offset)
    {
        if (pattern.empty()) return pattern;
        
        std::vector<bool> rotated(pattern.size());
        int size = static_cast<int>(pattern.size());
        
        // Normalize offset to positive range
        offset = ((offset % size) + size) % size;
        
        for (int i = 0; i < size; ++i) {
            int newIndex = (i + offset) % size;
            rotated[newIndex] = pattern[i];
        }
        
        return rotated;
    }
};

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
class ProgressiveOffsetTester
{
public:
    void runAllTests()
    {
        std::cout << "=== PROGRESSIVE OFFSET PATTERN TESTS ===" << std::endl;
        
        testBasicProgressiveOffset();
        testNegativeProgressiveOffset();
        testZeroProgressiveOffset();
        testLargeProgressiveOffset();
        testProgressiveOffsetTriggerCount();
        testProgressiveOffsetWrapping();
        testProgressiveOffsetEdgeCases();
        
        std::cout << "\n✅ All progressive offset tests passed!" << std::endl;
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
    
    void testBasicProgressiveOffset()
    {
        std::cout << "\n--- Testing Basic Progressive Offset (+2) ---" << std::endl;
        
        TestPatternEngine engine;
        
        // Generate E(5,8) pattern
        auto euclideanPattern = generateEuclidean(5, 8);
        engine.setPattern(euclideanPattern);
        engine.setProgressiveOffset(true, 0, 2); // +2 offset per trigger
        
        auto basePattern = engine.getCurrentPattern();
        int baseOnsets = countOnsets(basePattern);
        
        std::cout << "Base E(5,8):      " << patternToString(basePattern) << " (offset 0)" << std::endl;
        std::cout << "Base onset count: " << baseOnsets << std::endl;
        
        // Verify initial state
        assert(engine.getCurrentOffset() == 0);
        assert(engine.getProgressiveTriggerCount() == 0);
        
        // Store patterns to verify they're different
        std::vector<std::vector<bool>> triggeredPatterns;
        
        // Test progressive triggers
        for (int i = 1; i <= 3; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            assert(engine.getCurrentOffset() == i * 2);
            
            std::cout << "After trigger " << i << ":  " << patternToString(currentPattern) 
                     << " (offset " << (i * 2) << ")" << std::endl;
                     
            // Verify onset count remains constant
            assert(countOnsets(currentPattern) == baseOnsets);
            
            // Verify trigger count is correct
            assert(engine.getProgressiveTriggerCount() == i);
            
            // Store pattern for uniqueness check
            triggeredPatterns.push_back(currentPattern);
        }
        
        // Verify that patterns are actually different (unless wrap-around occurs)
        if (triggeredPatterns.size() >= 2) {
            // At least some patterns should be different unless we have a trivial case
            bool foundDifferentPattern = false;
            for (size_t i = 0; i < triggeredPatterns.size(); ++i) {
                if (triggeredPatterns[i] != basePattern) {
                    foundDifferentPattern = true;
                    break;
                }
            }
            // For a non-trivial pattern with +2 offset, we should see changes
            if (baseOnsets > 1 && baseOnsets < 7) { // Non-trivial case
                assert(foundDifferentPattern);
            }
        }
        
        std::cout << "✅ Basic progressive offset test passed" << std::endl;
    }
    
    void testNegativeProgressiveOffset()
    {
        std::cout << "\n--- Testing Negative Progressive Offset (-1) ---" << std::endl;
        
        TestPatternEngine engine;
        
        // E(3,8) = 10010010
        auto euclideanPattern = generateEuclidean(3, 8);
        engine.setPattern(euclideanPattern);
        engine.setProgressiveOffset(true, 0, -1); // -1 offset per trigger
        
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base E(3,8):      " << patternToString(basePattern) << " (offset 0)" << std::endl;
        
        // Test negative progression
        for (int i = 1; i <= 3; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            assert(engine.getCurrentOffset() == -i);
            
            std::cout << "After trigger " << i << ":  " << patternToString(currentPattern) 
                     << " (offset " << (-i) << ")" << std::endl;
            
            // Verify onset count remains constant
            assert(countOnsets(currentPattern) == countOnsets(basePattern));
            
            // Pattern should be different from base
            assert(currentPattern != basePattern);
        }
        
        std::cout << "✅ Negative progressive offset test passed" << std::endl;
    }
    
    void testZeroProgressiveOffset()
    {
        std::cout << "\n--- Testing Zero Progressive Offset (+0) ---" << std::endl;
        
        TestPatternEngine engine;
        
        auto testPattern = generateEuclidean(3, 8);
        engine.setPattern(testPattern);
        engine.setProgressiveOffset(true, 0, 0); // No offset change
        
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base pattern:     " << patternToString(basePattern) << std::endl;
        
        // Multiple triggers should produce identical patterns
        for (int i = 1; i <= 5; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            assert(currentPattern == basePattern);
            assert(engine.getCurrentOffset() == 0);
        }
        
        std::cout << "All triggers produced identical patterns (as expected)" << std::endl;
        std::cout << "✅ Zero progressive offset test passed" << std::endl;
    }
    
    void testLargeProgressiveOffset()
    {
        std::cout << "\n--- Testing Large Progressive Offset (+5) ---" << std::endl;
        
        TestPatternEngine engine;
        
        auto testPattern = generateEuclidean(3, 8);
        engine.setPattern(testPattern);
        engine.setProgressiveOffset(true, 0, 5); // +5 offset per trigger
        
        auto basePattern = engine.getCurrentPattern();
        int baseOnsets = countOnsets(basePattern);
        std::cout << "Base pattern:     " << patternToString(basePattern) << " (offset 0)" << std::endl;
        
        // Test large offset progression
        for (int i = 1; i <= 3; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            int expectedOffset = i * 5;
            assert(engine.getCurrentOffset() == expectedOffset);
            
            std::cout << "After trigger " << i << ":  " << patternToString(currentPattern) 
                     << " (offset " << expectedOffset << " = " << (expectedOffset % 8) << " mod 8)" << std::endl;
            
            // Verify onset count remains constant
            assert(countOnsets(currentPattern) == baseOnsets);
        }
        
        std::cout << "✅ Large progressive offset test passed" << std::endl;
    }
    
    void testProgressiveOffsetTriggerCount()
    {
        std::cout << "\n--- Testing Progressive Offset Trigger Count Tracking ---" << std::endl;
        
        TestPatternEngine engine;
        
        std::vector<bool> testPattern = {true, false, true, false, false, false, false, false};
        engine.setPattern(testPattern);
        engine.setProgressiveOffset(true, 0, 3); // +3 offset per trigger
        
        // Initial state
        assert(engine.getProgressiveTriggerCount() == 0);
        assert(engine.getCurrentOffset() == 0);
        std::cout << "Initial: trigger count = 0, offset = 0" << std::endl;
        
        // Test trigger count progression
        for (int i = 1; i <= 5; ++i) {
            engine.triggerProgressiveOffset();
            assert(engine.getProgressiveTriggerCount() == i);
            assert(engine.getCurrentOffset() == i * 3);
            std::cout << "Trigger " << i << ": trigger count = " << i << ", offset = " << (i * 3) << std::endl;
        }
        
        std::cout << "✅ Progressive offset trigger count test passed" << std::endl;
    }
    
    void testProgressiveOffsetWrapping()
    {
        std::cout << "\n--- Testing Progressive Offset Wraparound ---" << std::endl;
        
        TestPatternEngine engine;
        
        // 4-step pattern to test wrapping
        std::vector<bool> testPattern = {true, true, false, false};
        engine.setPattern(testPattern);
        engine.setProgressiveOffset(true, 0, 3); // +3 offset per trigger
        
        std::cout << "Base 4-step pattern: " << patternToString(testPattern) << std::endl;
        
        // Test wraparound behavior
        for (int i = 1; i <= 8; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            
            std::cout << "Trigger " << i << " (+3 offset): " << patternToString(currentPattern) 
                     << " (total offset: " << (i * 3) << " = " << ((i * 3) % 4) << " mod 4)" << std::endl;
            
            // Verify onset count remains constant
            assert(countOnsets(currentPattern) == countOnsets(testPattern));
        }
        
        std::cout << "✅ Progressive offset wraparound test passed" << std::endl;
    }
    
    void testProgressiveOffsetEdgeCases()
    {
        std::cout << "\n--- Testing Progressive Offset Edge Cases ---" << std::endl;
        
        struct TestCase {
            std::vector<bool> pattern;
            int offset;
            std::string description;
        };
        
        std::vector<TestCase> edgeCases = {
            {{true}, 1, "Single onset pattern"},
            {{false}, 2, "Single empty step"},
            {{true, true, true, true}, 1, "All onsets"},
            {{false, false, false, false}, 3, "No onsets"},
            {{true, false}, 1, "Minimal 2-step pattern"},
            {generateEuclidean(1, 16), 8, "Large sparse pattern"},
            {generateEuclidean(15, 16), 1, "Dense pattern"}
        };
        
        for (const auto& testCase : edgeCases) {
            std::cout << "\nTesting: " << testCase.description << std::endl;
            std::cout << "Pattern: " << patternToString(testCase.pattern) << std::endl;
            
            TestPatternEngine engine;
            engine.setPattern(testCase.pattern);
            engine.setProgressiveOffset(true, 0, testCase.offset);
            
            int baseOnsets = countOnsets(testCase.pattern);
            
            // Test multiple triggers don't break anything
            for (int i = 1; i <= 3; ++i) {
                engine.triggerProgressiveOffset();
                auto currentPattern = engine.getCurrentPattern();
                
                // Verify onset count remains constant
                assert(countOnsets(currentPattern) == baseOnsets);
                
                // Verify trigger count tracks correctly
                assert(engine.getProgressiveTriggerCount() == i);
                assert(engine.getCurrentOffset() == i * testCase.offset);
            }
            
            std::cout << "  ✓ Edge case handled correctly" << std::endl;
        }
        
        std::cout << "✅ Progressive offset edge cases test passed" << std::endl;
    }
};

//==============================================================================
int main()
{
    try {
        ProgressiveOffsetTester tester;
        tester.runAllTests();
        
        std::cout << "\n🎉 ALL PROGRESSIVE OFFSET TESTS COMPLETED SUCCESSFULLY! 🎉" << std::endl;
        std::cout << "\nKey features verified:" << std::endl;
        std::cout << "  ✓ Basic progressive offset (+N)" << std::endl;
        std::cout << "  ✓ Negative progressive offset (-N)" << std::endl;
        std::cout << "  ✓ Zero progressive offset (no change)" << std::endl;
        std::cout << "  ✓ Large offset values with wraparound" << std::endl;
        std::cout << "  ✓ Trigger count tracking" << std::endl;
        std::cout << "  ✓ Pattern wraparound behavior" << std::endl;
        std::cout << "  ✓ Edge cases (single onset, no onsets, dense patterns)" << std::endl;
        std::cout << "  ✓ Onset count preservation during rotation" << std::endl;
        
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