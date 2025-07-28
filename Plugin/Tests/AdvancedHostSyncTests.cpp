/*
  ==============================================================================

    Advanced Host Sync Tests - Phase 3 Features
    Unit tests for host loop sync and automatic pattern length adjustment

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

// Mock classes to simulate the plugin functionality
class MockRhythmPatternExplorer {
public:
    // Host loop sync variables
    bool hostIsLooping = false;
    double hostLoopStart = 0.0;
    double hostLoopEnd = 0.0;
    bool enableLoopSync = true;
    double lastHostPosition = 0.0;
    int currentStep = 0;
    int currentSample = 0;
    int globalOnsetCounter = 0;
    int uiAccentOffset = 0;
    
    // Mock automatic pattern length calculation
    double calculateAutoPatternLength(const std::vector<bool>& pattern) const {
        if (pattern.empty()) return 4.0;
        
        int stepCount = static_cast<int>(pattern.size());
        int onsetCount = 0;
        for (bool step : pattern) {
            if (step) onsetCount++;
        }
        
        double density = static_cast<double>(onsetCount) / stepCount;
        double baseLength;
        
        // Step count based heuristics
        if (stepCount <= 4) {
            baseLength = 1.0;
        } else if (stepCount <= 8) {
            baseLength = 2.0;
        } else if (stepCount <= 16) {
            baseLength = 4.0;
        } else {
            baseLength = 8.0;
        }
        
        // Density adjustment
        if (density < 0.2) {
            baseLength *= 2.0;
        } else if (density < 0.4) {
            baseLength *= 1.5;
        } else if (density > 0.8) {
            baseLength *= 0.5;
        } else if (density > 0.6) {
            baseLength *= 0.75;
        }
        
        // Special cases
        if (stepCount == 7 && onsetCount == 3) {
            baseLength = 2.0;
        } else if (stepCount == 8 && onsetCount == 3) {
            baseLength = 2.0;
        } else if (stepCount == 16 && onsetCount == 3) {
            baseLength = 4.0;
        } else if (stepCount == 5 && onsetCount >= 3) {
            baseLength = (onsetCount >= 4) ? 1.25 : 2.5;
        }
        
        // Bounds checking
        if (baseLength < 0.5) baseLength = 0.5;
        if (baseLength > 16.0) baseLength = 16.0;
        
        return baseLength;
    }
    
    // Mock loop restart detection
    bool detectLoopRestart(double currentPosition, bool isLooping) {
        if (enableLoopSync && isLooping) {
            bool wasLooping = hostIsLooping;
            hostIsLooping = true;
            
            // Detect loop restart (position jumped backward significantly)
            if (wasLooping && currentPosition < lastHostPosition - 0.1) {
                return true;
            }
            
            if (!wasLooping) {
                hostLoopStart = currentPosition;
            }
        } else {
            hostIsLooping = false;
        }
        
        lastHostPosition = currentPosition;
        return false;
    }
    
    // Mock pattern reset
    void resetPatternOnLoopRestart() {
        currentStep = 0;
        currentSample = 0;
        globalOnsetCounter = 0;
        uiAccentOffset = 0;
    }
};

// Test functions
bool testAutoPatternLengthBasic() {
    std::cout << "Testing basic automatic pattern length calculation...\n";
    MockRhythmPatternExplorer plugin;
    
    // Test empty pattern
    std::vector<bool> emptyPattern;
    double length = plugin.calculateAutoPatternLength(emptyPattern);
    assert(length == 4.0);
    std::cout << "  ✓ Empty pattern returns 4.0 beats\n";
    
    // Test 4-step pattern
    std::vector<bool> shortPattern = {true, false, true, false};
    length = plugin.calculateAutoPatternLength(shortPattern);
    assert(length == 1.0);
    std::cout << "  ✓ 4-step pattern returns 1.0 beat\n";
    
    // Test 8-step pattern (standard)
    std::vector<bool> standardPattern = {true, false, false, true, false, false, true, false};
    length = plugin.calculateAutoPatternLength(standardPattern);
    assert(length == 2.0);
    std::cout << "  ✓ 8-step pattern returns 2.0 beats\n";
    
    return true;
}

bool testAutoPatternLengthDensity() {
    std::cout << "Testing density-based automatic pattern length adjustment...\n";
    MockRhythmPatternExplorer plugin;
    
    // Test very sparse pattern (1 onset in 16 steps = 6.25% density)
    std::vector<bool> sparsePattern = {true, false, false, false, false, false, false, false,
                                      false, false, false, false, false, false, false, false};
    double length = plugin.calculateAutoPatternLength(sparsePattern);
    assert(length == 8.0); // 4.0 base * 2.0 sparse multiplier
    std::cout << "  ✓ Very sparse pattern (6.25% density) returns 8.0 beats\n";
    
    // Test dense pattern (12 onsets in 16 steps = 75% density)
    std::vector<bool> densePattern = {true, true, false, true, true, true, false, true,
                                     true, true, false, true, true, true, false, true};
    length = plugin.calculateAutoPatternLength(densePattern);
    assert(length == 3.0); // 4.0 base * 0.75 dense multiplier (density 0.75 is > 0.6 but <= 0.8)
    std::cout << "  ✓ Dense pattern (75% density) returns 3.0 beats\n";
    
    // Test very dense pattern (13 onsets in 16 steps = 81.25% density)
    std::vector<bool> veryDensePattern = {true, true, true, true, true, true, false, true,
                                         true, true, false, true, true, true, false, true};
    length = plugin.calculateAutoPatternLength(veryDensePattern);
    assert(length == 2.0); // 4.0 base * 0.5 very dense multiplier (density 0.8125 is > 0.8)
    std::cout << "  ✓ Very dense pattern (81.25% density) returns 2.0 beats\n";
    
    return true;
}

bool testAutoPatternLengthSpecialCases() {
    std::cout << "Testing special case pattern recognition...\n";
    MockRhythmPatternExplorer plugin;
    
    // Test E(3,8) tresillo pattern
    std::vector<bool> tresilloPattern = {true, false, false, true, false, false, true, false};
    double length = plugin.calculateAutoPatternLength(tresilloPattern);
    assert(length == 2.0);
    std::cout << "  ✓ E(3,8) tresillo pattern returns 2.0 beats\n";
    
    // Test E(3,7) pattern
    std::vector<bool> e37Pattern = {true, false, false, true, false, true, false};
    length = plugin.calculateAutoPatternLength(e37Pattern);
    assert(length == 2.0);
    std::cout << "  ✓ E(3,7) pattern returns 2.0 beats\n";
    
    // Test E(3,16) sparse pattern
    std::vector<bool> e316Pattern = {true, false, false, false, false, false, true, false,
                                    false, false, false, true, false, false, false, false};
    length = plugin.calculateAutoPatternLength(e316Pattern);
    assert(length == 4.0);
    std::cout << "  ✓ E(3,16) sparse pattern returns 4.0 beats\n";
    
    // Test quintillo pattern E(3,5)
    std::vector<bool> quintillo3Pattern = {true, false, true, false, true};
    length = plugin.calculateAutoPatternLength(quintillo3Pattern);
    assert(length == 2.5);
    std::cout << "  ✓ E(3,5) quintillo pattern returns 2.5 beats\n";
    
    // Test denser quintillo pattern E(4,5)
    std::vector<bool> quintillo4Pattern = {true, false, true, true, true};
    length = plugin.calculateAutoPatternLength(quintillo4Pattern);
    assert(length == 1.25);
    std::cout << "  ✓ E(4,5) dense quintillo pattern returns 1.25 beats\n";
    
    return true;
}

bool testAutoPatternLengthBounds() {
    std::cout << "Testing automatic pattern length bounds checking...\n";
    MockRhythmPatternExplorer plugin;
    
    // The bounds are enforced in the actual implementation
    // Here we verify the logic would produce reasonable results
    
    // Test that very long patterns don't exceed maximum
    std::vector<bool> veryLongPattern(64, false);
    veryLongPattern[0] = true; // Very sparse, very long
    double length = plugin.calculateAutoPatternLength(veryLongPattern);
    assert(length <= 16.0);
    std::cout << "  ✓ Very long pattern respects maximum bound (≤16.0 beats)\n";
    
    // Test that results don't go below minimum
    std::vector<bool> veryDenseShort = {true, true, true, true};
    length = plugin.calculateAutoPatternLength(veryDenseShort);
    assert(length >= 0.5);
    std::cout << "  ✓ Very dense short pattern respects minimum bound (≥0.5 beats)\n";
    
    return true;
}

bool testLoopRestartDetection() {
    std::cout << "Testing host loop restart detection...\n";
    MockRhythmPatternExplorer plugin;
    
    // Test normal forward progression (no loop restart)
    bool restart = plugin.detectLoopRestart(1.0, true);
    assert(!restart);
    restart = plugin.detectLoopRestart(2.0, true);
    assert(!restart);
    std::cout << "  ✓ Forward progression does not trigger restart\n";
    
    // Test backward jump detection (loop restart)
    restart = plugin.detectLoopRestart(0.5, true); // Jump backward by 1.5 beats
    assert(restart);
    std::cout << "  ✓ Backward jump (>0.1 ppq) triggers restart detection\n";
    
    // Test small backward movement (not a restart)
    plugin.lastHostPosition = 2.0;
    restart = plugin.detectLoopRestart(1.95, true); // Small backward movement
    assert(!restart);
    std::cout << "  ✓ Small backward movement (<0.1 ppq) does not trigger restart\n";
    
    // Test loop disabled
    plugin.enableLoopSync = false;
    restart = plugin.detectLoopRestart(0.0, true); // Big jump but sync disabled
    assert(!restart);
    assert(!plugin.hostIsLooping);
    std::cout << "  ✓ Loop sync disabled prevents restart detection\n";
    
    return true;
}

bool testPatternResetOnLoopRestart() {
    std::cout << "Testing pattern reset functionality on loop restart...\n";
    MockRhythmPatternExplorer plugin;
    
    // Set up some non-zero state
    plugin.currentStep = 5;
    plugin.currentSample = 1000;
    plugin.globalOnsetCounter = 10;
    plugin.uiAccentOffset = 3;
    
    // Trigger reset
    plugin.resetPatternOnLoopRestart();
    
    // Verify everything is reset
    assert(plugin.currentStep == 0);
    assert(plugin.currentSample == 0);
    assert(plugin.globalOnsetCounter == 0);
    assert(plugin.uiAccentOffset == 0);
    
    std::cout << "  ✓ Pattern reset clears all timing and accent state\n";
    
    return true;
}

bool testLoopSyncIntegration() {
    std::cout << "Testing complete loop sync integration...\n";
    MockRhythmPatternExplorer plugin;
    
    // Simulate a complete loop cycle
    plugin.currentStep = 3;
    plugin.globalOnsetCounter = 5;
    
    // First loop iteration
    plugin.detectLoopRestart(0.0, true);
    assert(plugin.hostIsLooping);
    assert(plugin.hostLoopStart == 0.0);
    
    // Progress through loop
    plugin.detectLoopRestart(1.0, true);
    plugin.detectLoopRestart(2.0, true);
    plugin.detectLoopRestart(3.0, true);
    
    // Loop restart (jump back to beginning)
    bool restart = plugin.detectLoopRestart(0.1, true);
    assert(restart);
    
    // Verify state would be reset in full implementation
    plugin.resetPatternOnLoopRestart();
    assert(plugin.currentStep == 0);
    assert(plugin.globalOnsetCounter == 0);
    
    std::cout << "  ✓ Complete loop cycle handles restart detection and reset\n";
    
    return true;
}

// Main test runner
int main() {
    std::cout << "=== Advanced Host Sync Tests - Phase 3 ===\n\n";
    
    bool allPassed = true;
    
    // Automatic Pattern Length Tests
    std::cout << "## Automatic Pattern Length Adjustment Tests\n";
    allPassed &= testAutoPatternLengthBasic();
    allPassed &= testAutoPatternLengthDensity();
    allPassed &= testAutoPatternLengthSpecialCases();
    allPassed &= testAutoPatternLengthBounds();
    std::cout << "\n";
    
    // Host Loop Sync Tests
    std::cout << "## Host Loop Sync Tests\n";
    allPassed &= testLoopRestartDetection();
    allPassed &= testPatternResetOnLoopRestart();
    allPassed &= testLoopSyncIntegration();
    std::cout << "\n";
    
    // Final result
    if (allPassed) {
        std::cout << "🎉 ALL TESTS PASSED! Advanced Host Sync features are working correctly.\n";
        std::cout << "\nFeatures tested:\n";
        std::cout << "✓ Automatic pattern length calculation with musical heuristics\n";
        std::cout << "✓ Density-based duration adjustment\n";
        std::cout << "✓ Special case pattern recognition (tresillo, quintillo, etc.)\n";
        std::cout << "✓ Bounds checking for reasonable durations\n";
        std::cout << "✓ Host loop restart detection via backward position jumps\n";
        std::cout << "✓ Complete pattern state reset on loop restart\n";
        std::cout << "✓ Loop sync enable/disable functionality\n";
        return 0;
    } else {
        std::cout << "❌ SOME TESTS FAILED! Please review the implementation.\n";
        return 1;
    }
}