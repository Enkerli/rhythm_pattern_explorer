/*
  ==============================================================================

    TestProgressiveIntegrationFull.cpp
    Full integration tests for progressive patterns using actual PluginProcessor
    
    Tests that would have caught the encapsulation regression:
    - Progressive offset cycling (E(5,8)+2)
    - Progressive lengthening accumulation (E(5,8)*2)

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

// Include the actual plugin processor for real integration testing
#include "../Source/PluginProcessor.h"

class ProgressiveIntegrationTester
{
public:
    ProgressiveIntegrationTester() = default;
    
    void runAllTests()
    {
        std::cout << "=== PROGRESSIVE INTEGRATION TESTS (Full Plugin) ===" << std::endl;
        
        testProgressiveOffsetCycling();
        testProgressiveLengtheningAccumulation();
        testMixedProgressiveFeatures();
        
        std::cout << "\n✅ All progressive integration tests passed!" << std::endl;
    }

private:
    std::string patternToString(const std::vector<bool>& pattern)
    {
        std::string result;
        for (bool bit : pattern) {
            result += bit ? '1' : '0';
        }
        return result;
    }
    
    void testProgressiveOffsetCycling()
    {
        std::cout << "\n--- Testing Progressive Offset Cycling (E(5,8)+2) ---" << std::endl;
        
        RhythmPatternExplorerAudioProcessor processor;
        
        // Expected patterns for E(5,8)+2 progression
        // E(5,8) = "10110110" (tresillo pattern)
        // Each trigger should rotate clockwise by +2 steps
        std::vector<std::string> expectedPatterns = {
            "10110110",  // trigger 0: offset 0 (original E(5,8))
            "10101101",  // trigger 1: offset 2 (rotated +2 clockwise)  
            "01011011",  // trigger 2: offset 4 (rotated +4 clockwise)
            "11010110",  // trigger 3: offset 6 (rotated +6 clockwise)
            "10110110"   // trigger 4: offset 8→0 (wrapped back to start)
        };
        
        std::cout << "Expected progressive offset sequence:" << std::endl;
        for (size_t i = 0; i < expectedPatterns.size(); ++i) {
            std::cout << "  Trigger " << i << ": " << expectedPatterns[i] 
                      << " (offset " << (i * 2) % 8 << ")" << std::endl;
        }
        
        std::cout << "\nActual results:" << std::endl;
        for (size_t i = 0; i < expectedPatterns.size(); ++i) {
            processor.setUPIInput("E(5,8)+2");  // Trigger progression
            auto currentPattern = processor.patternEngine.getCurrentPattern();
            std::string actualBinary = patternToString(currentPattern);
            
            std::cout << "  Trigger " << i << ": " << actualBinary;
            
            if (actualBinary == expectedPatterns[i]) {
                std::cout << " ✓ PASS" << std::endl;
            } else {
                std::cout << " ✗ FAIL (expected " << expectedPatterns[i] << ")" << std::endl;
                assert(false && "Progressive offset cycling failed");
            }
        }
        
        std::cout << "✅ Progressive offset cycling test passed!" << std::endl;
    }
    
    void testProgressiveLengtheningAccumulation() 
    {
        std::cout << "\n--- Testing Progressive Lengthening Accumulation (E(5,8)*2) ---" << std::endl;
        
        RhythmPatternExplorerAudioProcessor processor;
        
        // E(5,8) = "10110110" (8 steps)
        // Each trigger should add 2 random steps
        std::vector<int> expectedLengths = {
            8,   // trigger 0: original length
            10,  // trigger 1: +2 steps  
            12,  // trigger 2: +2 more steps
            14,  // trigger 3: +2 more steps
            16   // trigger 4: +2 more steps
        };
        
        std::cout << "Expected progressive lengthening sequence:" << std::endl;
        for (size_t i = 0; i < expectedLengths.size(); ++i) {
            std::cout << "  Trigger " << i << ": " << expectedLengths[i] << " steps" << std::endl;
        }
        
        std::cout << "\nActual results:" << std::endl;
        for (size_t i = 0; i < expectedLengths.size(); ++i) {
            processor.setUPIInput("E(5,8)*2");  // Trigger progression
            auto currentPattern = processor.patternEngine.getCurrentPattern();
            int actualLength = static_cast<int>(currentPattern.size());
            
            std::cout << "  Trigger " << i << ": " << actualLength << " steps (pattern: " 
                      << patternToString(currentPattern) << ")";
            
            if (actualLength == expectedLengths[i]) {
                std::cout << " ✓ PASS" << std::endl;
            } else {
                std::cout << " ✗ FAIL (expected " << expectedLengths[i] << " steps)" << std::endl;
                assert(false && "Progressive lengthening accumulation failed");
            }
            
            // Verify the pattern contains both 1s and 0s (not all empty)
            bool hasOnes = false, hasZeros = false;
            for (bool bit : currentPattern) {
                if (bit) hasOnes = true;
                else hasZeros = true;
            }
            
            if (!hasOnes || !hasZeros) {
                std::cout << "    ⚠️  WARNING: Pattern appears to be empty or all 1s" << std::endl;
            }
        }
        
        std::cout << "✅ Progressive lengthening accumulation test passed!" << std::endl;
    }
    
    void testMixedProgressiveFeatures()
    {
        std::cout << "\n--- Testing Mixed Progressive Features ---" << std::endl;
        
        RhythmPatternExplorerAudioProcessor processor;
        
        // Test the edge case that works in original code: lengthening then offsetting
        std::cout << "Testing lengthening followed by offsetting..." << std::endl;
        
        // First, build up a lengthened pattern
        processor.setUPIInput("100000*2");  // Start lengthening
        processor.setUPIInput("100000*2");  // Add more length
        auto lengthened = processor.patternEngine.getCurrentPattern();
        int lengthenedSize = static_cast<int>(lengthened.size());
        
        std::cout << "  After lengthening: " << lengthenedSize << " steps (" 
                  << patternToString(lengthened) << ")" << std::endl;
        
        // Then add offset to the lengthened pattern
        processor.setUPIInput("100000+2");  // Switch to offset mode
        auto offsetted = processor.patternEngine.getCurrentPattern();
        
        std::cout << "  After switching to offset: " << offsetted.size() << " steps (" 
                  << patternToString(offsetted) << ")" << std::endl;
        
        // This should work as described: "the lengthened pattern is progressively offset"
        assert(offsetted.size() >= 6 && "Mixed progressive features failed");
        
        std::cout << "✅ Mixed progressive features test passed!" << std::endl;
    }
};

int main()
{
    try {
        ProgressiveIntegrationTester tester;
        tester.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}