/*
  ==============================================================================

    TestProgressiveOffsets.cpp
    Comprehensive unit tests for progressive offset patterns (+N notation)
    
    Tests the progressive offset system that allows patterns to rotate by N steps
    each time they are triggered via MIDI, Enter key, or Tick button.

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

// Mock UPIParser and PatternEngine for testing
#include "../Source/UPIParser.h"
#include "../Source/PatternEngine.h"

//==============================================================================
class ProgressiveOffsetTester
{
public:
    ProgressiveOffsetTester() = default;
    
    void runAllTests()
    {
        std::cout << "=== PROGRESSIVE OFFSET PATTERN TESTS ===" << std::endl;
        
        testBasicProgressiveOffset();
        testNegativeProgressiveOffset();
        testZeroProgressiveOffset();
        testLargeProgressiveOffset();
        testEuclideanProgressiveOffsets();
        testBarlowProgressiveOffsets();
        testPolygonProgressiveOffsets();
        testRandomProgressiveOffsets();
        testBinaryProgressiveOffsets();
        testProgressiveOffsetWithAccents();
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
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test E(5,8)+2 progressive offset
        auto parseResult = parser.parsePattern("E(5,8)+2");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 2);
        assert(parseResult.initialOffset == 0);
        
        // Set up pattern engine
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        // Base pattern: E(5,8) = 10110110
        std::vector<bool> expectedBase = {true, false, true, true, false, true, true, false};
        assert(engine.getCurrentPattern() == expectedBase);
        assert(engine.getCurrentOffset() == 0);
        std::cout << "Base pattern:     " << patternToString(expectedBase) << " (offset 0)" << std::endl;
        
        // First trigger: rotate by +2
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == 2);
        std::cout << "After trigger 1:  " << patternToString(pattern1) << " (offset 2)" << std::endl;
        
        // Second trigger: rotate by +4 total
        engine.triggerProgressiveOffset();
        auto pattern2 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == 4);
        std::cout << "After trigger 2:  " << patternToString(pattern2) << " (offset 4)" << std::endl;
        
        // Third trigger: rotate by +6 total
        engine.triggerProgressiveOffset();
        auto pattern3 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == 6);
        std::cout << "After trigger 3:  " << patternToString(pattern3) << " (offset 6)" << std::endl;
        
        // Verify all patterns have same onset count
        assert(countOnsets(expectedBase) == countOnsets(pattern1));
        assert(countOnsets(pattern1) == countOnsets(pattern2));
        assert(countOnsets(pattern2) == countOnsets(pattern3));
        
        std::cout << "✅ Basic progressive offset test passed" << std::endl;
    }
    
    void testNegativeProgressiveOffset()
    {
        std::cout << "\n--- Testing Negative Progressive Offset (+-1) ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test E(3,8)+-1 (counterclockwise rotation)
        auto parseResult = parser.parsePattern("E(3,8)+-1");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == -1);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        // Base pattern: E(3,8) = 10010010
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base pattern:     " << patternToString(basePattern) << " (offset 0)" << std::endl;
        
        // First trigger: rotate by -1
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == -1);
        std::cout << "After trigger 1:  " << patternToString(pattern1) << " (offset -1)" << std::endl;
        
        // Second trigger: rotate by -2 total
        engine.triggerProgressiveOffset();
        auto pattern2 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == -2);
        std::cout << "After trigger 2:  " << patternToString(pattern2) << " (offset -2)" << std::endl;
        
        // Verify patterns are different but have same onset count
        assert(basePattern != pattern1);
        assert(pattern1 != pattern2);
        assert(countOnsets(basePattern) == countOnsets(pattern1));
        assert(countOnsets(pattern1) == countOnsets(pattern2));
        
        std::cout << "✅ Negative progressive offset test passed" << std::endl;
    }
    
    void testZeroProgressiveOffset()
    {
        std::cout << "\n--- Testing Zero Progressive Offset (+0) ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test E(3,8)+0 (no rotation)
        auto parseResult = parser.parsePattern("E(3,8)+0");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 0);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base pattern:     " << patternToString(basePattern) << std::endl;
        
        // Multiple triggers should produce identical patterns
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        assert(pattern1 == basePattern);
        assert(engine.getCurrentOffset() == 0);
        
        engine.triggerProgressiveOffset();
        auto pattern2 = engine.getCurrentPattern();
        assert(pattern2 == basePattern);
        assert(engine.getCurrentOffset() == 0);
        
        std::cout << "✅ Zero progressive offset test passed" << std::endl;
    }
    
    void testLargeProgressiveOffset()
    {
        std::cout << "\n--- Testing Large Progressive Offset (+5) ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test E(3,8)+5 (large rotation)
        auto parseResult = parser.parsePattern("E(3,8)+5");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 5);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base pattern:     " << patternToString(basePattern) << " (offset 0)" << std::endl;
        
        // First trigger: rotate by +5
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == 5);
        std::cout << "After trigger 1:  " << patternToString(pattern1) << " (offset 5)" << std::endl;
        
        // Second trigger: rotate by +10 total (wraps around 8-step pattern)
        engine.triggerProgressiveOffset();
        auto pattern2 = engine.getCurrentPattern();
        assert(engine.getCurrentOffset() == 10);
        std::cout << "After trigger 2:  " << patternToString(pattern2) << " (offset 10 = 2 mod 8)" << std::endl;
        
        std::cout << "✅ Large progressive offset test passed" << std::endl;
    }
    
    void testEuclideanProgressiveOffsets()
    {
        std::cout << "\n--- Testing Euclidean Progressive Offsets ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test various Euclidean patterns with progressive offsets
        std::vector<std::string> testPatterns = {
            "E(1,4)+1",   // Simple beat with +1 offset
            "E(2,5)+2",   // Asymmetric pattern
            "E(4,9)+3",   // Complex Euclidean
            "E(7,16)+4"   // Dense pattern
        };
        
        for (const auto& patternStr : testPatterns) {
            std::cout << "\nTesting: " << patternStr << std::endl;
            
            auto parseResult = parser.parsePattern(patternStr);
            assert(parseResult.isValid());
            assert(parseResult.hasProgressiveOffset);
            
            engine.setPattern(parseResult.pattern);
            engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
            
            auto basePattern = engine.getCurrentPattern();
            int baseOnsets = countOnsets(basePattern);
            
            // Test multiple triggers
            for (int i = 1; i <= 3; ++i) {
                engine.triggerProgressiveOffset();
                auto currentPattern = engine.getCurrentPattern();
                
                // Onset count should remain constant
                assert(countOnsets(currentPattern) == baseOnsets);
                
                // Pattern should rotate correctly
                int expectedOffset = parseResult.progressiveOffset * i;
                assert(engine.getCurrentOffset() == expectedOffset);
                
                std::cout << "  Trigger " << i << ": " << patternToString(currentPattern) 
                         << " (offset " << expectedOffset << ")" << std::endl;
            }
        }
        
        std::cout << "✅ Euclidean progressive offset tests passed" << std::endl;
    }
    
    void testBarlowProgressiveOffsets()
    {
        std::cout << "\n--- Testing Barlow Progressive Offsets ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test B(3,8)+1
        auto parseResult = parser.parsePattern("B(3,8)+1");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 1);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        auto basePattern = engine.getCurrentPattern();
        int baseOnsets = countOnsets(basePattern);
        std::cout << "Base B(3,8):      " << patternToString(basePattern) << std::endl;
        
        // Multiple triggers
        for (int i = 1; i <= 3; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            assert(countOnsets(currentPattern) == baseOnsets);
            std::cout << "Trigger " << i << ":       " << patternToString(currentPattern) 
                     << " (offset " << i << ")" << std::endl;
        }
        
        std::cout << "✅ Barlow progressive offset test passed" << std::endl;
    }
    
    void testPolygonProgressiveOffsets()
    {
        std::cout << "\n--- Testing Polygon Progressive Offsets ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test P(4,12)+3
        auto parseResult = parser.parsePattern("P(4,12)+3");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 3);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base P(4,12):     " << patternToString(basePattern) << std::endl;
        
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        std::cout << "After trigger:    " << patternToString(pattern1) << " (offset 3)" << std::endl;
        
        // Should maintain same number of onsets
        assert(countOnsets(basePattern) == countOnsets(pattern1));
        
        std::cout << "✅ Polygon progressive offset test passed" << std::endl;
    }
    
    void testRandomProgressiveOffsets()
    {
        std::cout << "\n--- Testing Random Progressive Offsets ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test R(3,8)+2 - random patterns with progressive offset
        auto parseResult = parser.parsePattern("R(3,8)+2");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 2);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        auto basePattern = engine.getCurrentPattern();
        int baseOnsets = countOnsets(basePattern);
        std::cout << "Base R(3,8):      " << patternToString(basePattern) << std::endl;
        
        // Multiple triggers - onset count should remain constant
        for (int i = 1; i <= 3; ++i) {
            engine.triggerProgressiveOffset();
            auto currentPattern = engine.getCurrentPattern();
            assert(countOnsets(currentPattern) == baseOnsets);
            std::cout << "Trigger " << i << ":       " << patternToString(currentPattern) << std::endl;
        }
        
        std::cout << "✅ Random progressive offset test passed" << std::endl;
    }
    
    void testBinaryProgressiveOffsets()
    {
        std::cout << "\n--- Testing Binary Progressive Offsets ---" << std::endl;
        
        UPIParser parser;
        PatternEngine engine;
        
        // Test binary pattern with progressive offset
        auto parseResult = parser.parsePattern("10101010+1");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.progressiveOffset == 1);
        
        engine.setPattern(parseResult.pattern);
        engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
        
        auto basePattern = engine.getCurrentPattern();
        std::cout << "Base 10101010:    " << patternToString(basePattern) << std::endl;
        
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        std::cout << "After trigger:    " << patternToString(pattern1) << " (should be 01010101)" << std::endl;
        
        // Should be rotated by 1 step
        std::vector<bool> expected = {false, true, false, true, false, true, false, true};
        assert(pattern1 == expected);
        
        std::cout << "✅ Binary progressive offset test passed" << std::endl;
    }
    
    void testProgressiveOffsetWithAccents()
    {
        std::cout << "\n--- Testing Progressive Offset with Accent Patterns ---" << std::endl;
        
        UPIParser parser;
        
        // Test accent pattern with progressive offset
        auto parseResult = parser.parsePattern("{10}E(3,8)+2");
        assert(parseResult.isValid());
        assert(parseResult.hasProgressiveOffset);
        assert(parseResult.hasAccentPattern);
        assert(parseResult.progressiveOffset == 2);
        
        std::cout << "Base pattern:     " << patternToString(parseResult.pattern) << std::endl;
        std::cout << "Accent pattern:   " << patternToString(parseResult.accentPattern) << std::endl;
        
        // Pattern should parse correctly with both features
        assert(countOnsets(parseResult.pattern) == 3); // E(3,8) has 3 onsets
        assert(parseResult.accentPattern.size() == 2); // {10} has 2 elements
        
        std::cout << "✅ Progressive offset with accents test passed" << std::endl;
    }
    
    void testProgressiveOffsetTriggerCount()
    {
        std::cout << "\n--- Testing Progressive Offset Trigger Count Tracking ---" << std::endl;
        
        PatternEngine engine;
        
        // Set up progressive offset
        std::vector<bool> testPattern = {true, false, true, false, false, false, false, false};
        engine.setPattern(testPattern);
        engine.setProgressiveOffset(true, 0, 3); // +3 offset per trigger
        
        // Initial state
        assert(engine.getProgressiveTriggerCount() == 0);
        assert(engine.getCurrentOffset() == 0);
        
        // First trigger
        engine.triggerProgressiveOffset();
        assert(engine.getProgressiveTriggerCount() == 1);
        assert(engine.getCurrentOffset() == 3);
        
        // Second trigger
        engine.triggerProgressiveOffset();
        assert(engine.getProgressiveTriggerCount() == 2);
        assert(engine.getCurrentOffset() == 6);
        
        // Third trigger
        engine.triggerProgressiveOffset();
        assert(engine.getProgressiveTriggerCount() == 3);
        assert(engine.getCurrentOffset() == 9);
        
        std::cout << "Trigger count tracking works correctly" << std::endl;
        std::cout << "✅ Progressive offset trigger count test passed" << std::endl;
    }
    
    void testProgressiveOffsetWrapping()
    {
        std::cout << "\n--- Testing Progressive Offset Wraparound ---" << std::endl;
        
        PatternEngine engine;
        
        // 4-step pattern with +3 offset (should wrap)
        std::vector<bool> testPattern = {true, true, false, false};
        engine.setPattern(testPattern);
        engine.setProgressiveOffset(true, 0, 3);
        
        std::cout << "Base 4-step pattern: " << patternToString(testPattern) << std::endl;
        
        // First trigger: rotate by +3
        engine.triggerProgressiveOffset();
        auto pattern1 = engine.getCurrentPattern();
        std::cout << "After +3 offset:     " << patternToString(pattern1) << std::endl;
        
        // Second trigger: rotate by +6 (wraps around 4-step pattern)
        engine.triggerProgressiveOffset();
        auto pattern2 = engine.getCurrentPattern();
        std::cout << "After +6 offset:     " << patternToString(pattern2) << std::endl;
        
        // Verify wrapping works correctly
        assert(countOnsets(testPattern) == countOnsets(pattern1));
        assert(countOnsets(pattern1) == countOnsets(pattern2));
        
        std::cout << "✅ Progressive offset wraparound test passed" << std::endl;
    }
    
    void testProgressiveOffsetEdgeCases()
    {
        std::cout << "\n--- Testing Progressive Offset Edge Cases ---" << std::endl;
        
        UPIParser parser;
        
        // Test edge case patterns
        std::vector<std::string> edgeCases = {
            "1+1",        // Single onset
            "0+2",        // No onsets
            "1111+4",     // All onsets
            "E(1,16)+8",  // Large pattern
            "E(8,8)+1"    // Dense pattern
        };
        
        for (const auto& pattern : edgeCases) {
            std::cout << "\nTesting edge case: " << pattern << std::endl;
            
            auto parseResult = parser.parsePattern(pattern);
            assert(parseResult.isValid());
            assert(parseResult.hasProgressiveOffset);
            
            PatternEngine engine;
            engine.setPattern(parseResult.pattern);
            engine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
            
            int baseOnsets = countOnsets(parseResult.pattern);
            
            // Test multiple triggers don't break anything
            for (int i = 0; i < 3; ++i) {
                engine.triggerProgressiveOffset();
                auto currentPattern = engine.getCurrentPattern();
                assert(countOnsets(currentPattern) == baseOnsets);
            }
            
            std::cout << "  Edge case handled correctly" << std::endl;
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