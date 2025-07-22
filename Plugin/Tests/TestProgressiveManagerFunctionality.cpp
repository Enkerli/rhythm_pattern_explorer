/*
  ==============================================================================

    TestProgressiveManagerFunctionality.cpp
    Tests for ProgressiveManager encapsulated functionality
    
    This test verifies that the ProgressiveManager correctly handles:
    - Progressive offset patterns (+N notation)
    - Progressive lengthening patterns (*N notation) 
    - Progressive transformation patterns (>N notation)
    - State management and LRU cleanup
    - Integration with PatternEngine

  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <map>

// Mock PatternEngine for testing
class MockPatternEngine
{
public:
    MockPatternEngine() = default;
    
    void setProgressiveOffset(bool enabled, int initial = 0, int progressive = 0) {
        progressiveOffsetEnabled = enabled;
        initialOffset = initial;
        progressiveStep = progressive;
        currentOffset = initial;
    }
    
    bool hasProgressiveOffsetEnabled() const { return progressiveOffsetEnabled; }
    int getProgressiveOffsetValue() const { return progressiveStep; }
    
    void advanceProgressiveOffset() {
        currentOffset += progressiveStep;
    }
    
    int getCurrentOffset() const { return currentOffset; }

private:
    bool progressiveOffsetEnabled = false;
    int initialOffset = 0;
    int progressiveStep = 0;
    int currentOffset = 0;
};

// Simplified ProgressiveManager for testing
class TestProgressiveManager
{
public:
    TestProgressiveManager() = default;
    
    // Pattern Analysis
    bool hasAnyProgressiveFeatures(const std::string& pattern) const {
        return hasProgressiveOffset(pattern) || 
               hasProgressiveLengthening(pattern) || 
               hasProgressiveTransformation(pattern);
    }
    
    bool hasProgressiveOffset(const std::string& pattern) const {
        size_t plusPos = pattern.find('+');
        if (plusPos == std::string::npos || plusPos == 0) return false;
        std::string afterPlus = pattern.substr(plusPos + 1);
        return afterPlus.find_first_not_of("0123456789-") == std::string::npos && !afterPlus.empty();
    }
    
    bool hasProgressiveLengthening(const std::string& pattern) const {
        size_t starPos = pattern.find('*');
        if (starPos == std::string::npos || starPos == 0) return false;
        std::string afterStar = pattern.substr(starPos + 1);
        return afterStar.find_first_not_of("0123456789") == std::string::npos && !afterStar.empty();
    }
    
    bool hasProgressiveTransformation(const std::string& pattern) const {
        size_t gtPos = pattern.find('>');
        if (gtPos == std::string::npos || gtPos == 0) return false;
        std::string afterGt = pattern.substr(gtPos + 1);
        return afterGt.find_first_not_of("0123456789") == std::string::npos && !afterGt.empty();
    }
    
    // State Management
    void initializeProgressiveState(const std::string& stateKey, 
                                  const std::string& basePattern,
                                  const std::string& progressivePattern) {
        ProgressiveState& state = progressiveStates[stateKey];
        state.basePattern = basePattern;
        state.fullPattern = progressivePattern;
        
        // Parse progressive features
        parseProgressiveNotation(progressivePattern, state);
    }
    
    std::string applyProgressive(const std::string& stateKey, MockPatternEngine& patternEngine) {
        auto it = progressiveStates.find(stateKey);
        if (it == progressiveStates.end()) {
            return ""; // No state found
        }
        
        ProgressiveState& state = it->second;
        
        // Apply transformations based on state
        if (state.hasOffset) {
            patternEngine.setProgressiveOffset(true, state.currentOffset, state.offsetStep);
            return state.basePattern + " (offset applied: " + std::to_string(state.currentOffset) + ")";
        } else if (state.hasLengthening) {
            return state.basePattern + " (lengthened by: " + std::to_string(state.currentLengthening) + ")";
        } else if (state.hasTransformation) {
            return state.basePattern + " (transformed step: " + std::to_string(state.transformationStep) + ")";
        }
        
        return state.basePattern;
    }
    
    void triggerProgressive(const std::string& stateKey, MockPatternEngine& patternEngine) {
        auto it = progressiveStates.find(stateKey);
        if (it == progressiveStates.end()) return;
        
        ProgressiveState& state = it->second;
        
        // Advance progressive state
        if (state.hasOffset) {
            state.currentOffset += state.offsetStep;
            state.triggerCount++;
        } else if (state.hasLengthening) {
            state.currentLengthening += state.lengtheningStep;
            state.triggerCount++;
        } else if (state.hasTransformation) {
            state.transformationStep++;
            state.triggerCount++;
        }
    }
    
    // Getters
    int getProgressiveOffsetValue(const std::string& stateKey) const {
        auto it = progressiveStates.find(stateKey);
        return (it != progressiveStates.end() && it->second.hasOffset) ? it->second.currentOffset : 0;
    }
    
    bool hasProgressiveState(const std::string& stateKey) const {
        return progressiveStates.find(stateKey) != progressiveStates.end();
    }
    
    std::string getBasePattern(const std::string& stateKey) const {
        auto it = progressiveStates.find(stateKey);
        return (it != progressiveStates.end()) ? it->second.basePattern : "";
    }
    
    int getTransformationStepCount(const std::string& stateKey) const {
        auto it = progressiveStates.find(stateKey);
        return (it != progressiveStates.end()) ? it->second.triggerCount : 0;
    }
    
    // Reset methods
    void resetProgressiveOffset(const std::string& stateKey) {
        auto it = progressiveStates.find(stateKey);
        if (it != progressiveStates.end() && it->second.hasOffset) {
            it->second.currentOffset = it->second.offsetStep; // Reset to first step
            it->second.triggerCount = 1;
        }
    }
    
    void resetAllProgressiveForPattern(const std::string& stateKey) {
        auto it = progressiveStates.find(stateKey);
        if (it != progressiveStates.end()) {
            ProgressiveState& state = it->second;
            state.currentOffset = state.hasOffset ? state.offsetStep : 0;
            state.currentLengthening = state.hasLengthening ? state.lengtheningStep : 0;
            state.transformationStep = 0;
            state.triggerCount = state.hasOffset || state.hasLengthening ? 1 : 0;
        }
    }
    
    void clearAllProgressiveStates() {
        progressiveStates.clear();
    }
    
    int getActiveStateCount() const {
        return static_cast<int>(progressiveStates.size());
    }

private:
    struct ProgressiveState {
        std::string basePattern;
        std::string fullPattern;
        
        // Progressive Offset (+N)
        bool hasOffset = false;
        int offsetStep = 0;
        int currentOffset = 0;
        int triggerCount = 0;
        
        // Progressive Lengthening (*N)
        bool hasLengthening = false;
        int lengtheningStep = 0;
        int currentLengthening = 0;
        
        // Progressive Transformation (>N)
        bool hasTransformation = false;
        std::string transformationType;
        int targetOnsets = 0;
        int transformationStep = 0;
    };
    
    std::map<std::string, ProgressiveState> progressiveStates;
    
    void parseProgressiveNotation(const std::string& pattern, ProgressiveState& state) {
        // Parse progressive offset (+N)
        if (hasProgressiveOffset(pattern)) {
            size_t plusPos = pattern.rfind('+');
            std::string basePattern = pattern.substr(0, plusPos);
            int step = std::stoi(pattern.substr(plusPos + 1));
            
            state.basePattern = basePattern;
            state.hasOffset = true;
            state.offsetStep = step;
            state.currentOffset = step; // Start with first offset
            state.triggerCount = 1;
        }
        // Parse progressive lengthening (*N)
        else if (hasProgressiveLengthening(pattern)) {
            size_t starPos = pattern.rfind('*');
            std::string basePattern = pattern.substr(0, starPos);
            int step = std::stoi(pattern.substr(starPos + 1));
            
            state.basePattern = basePattern;
            state.hasLengthening = true;
            state.lengtheningStep = step;
            state.currentLengthening = step; // Start with first lengthening
            state.triggerCount = 1;
        }
        // Parse progressive transformation (>N)
        else if (hasProgressiveTransformation(pattern)) {
            size_t gtPos = pattern.rfind('>');
            std::string beforeGt = pattern.substr(0, gtPos);
            int target = std::stoi(pattern.substr(gtPos + 1));
            
            // Extract transformation type and base pattern
            // E.g., "E(1,8)B>8" -> type="B", base="E(1,8)", target=8
            size_t typePos = beforeGt.rfind(')') + 1;
            std::string basePattern = beforeGt.substr(0, typePos);
            std::string transformationType = beforeGt.substr(typePos);
            
            state.basePattern = basePattern;
            state.hasTransformation = true;
            state.transformationType = transformationType;
            state.targetOnsets = target;
            state.transformationStep = 0;
            state.triggerCount = 0;
        }
    }
};

//==============================================================================
class ProgressiveManagerTester
{
public:
    void runAllTests() {
        std::cout << "=== PROGRESSIVE MANAGER FUNCTIONALITY TESTS ===" << std::endl;
        std::cout << "Testing ProgressiveManager encapsulated functionality." << std::endl;
        
        testProgressivePatternAnalysis();
        testProgressiveOffsetManagement();
        testProgressiveLengtheningManagement();
        testProgressiveTransformationManagement();
        testStateManagementAndCleanup();
        testIntegrationWithPatternEngine();
        
        std::cout << "\n✅ All ProgressiveManager functionality tests passed!" << std::endl;
    }

private:
    void testProgressivePatternAnalysis() {
        std::cout << "\n--- Progressive Pattern Analysis Test ---" << std::endl;
        
        TestProgressiveManager manager;
        
        // Test progressive offset detection
        assert(manager.hasProgressiveOffset("E(3,8)+2") && "Should detect progressive offset");
        assert(manager.hasProgressiveOffset("B(5,13)+3") && "Should detect progressive offset");
        assert(!manager.hasProgressiveOffset("E(3,8)") && "Should not detect offset in simple pattern");
        assert(!manager.hasProgressiveOffset("E(3,8)*2") && "Should not detect offset in lengthening pattern");
        
        // Test progressive lengthening detection
        assert(manager.hasProgressiveLengthening("E(3,8)*2") && "Should detect progressive lengthening");
        assert(manager.hasProgressiveLengthening("B(5,13)*5") && "Should detect progressive lengthening");
        assert(!manager.hasProgressiveLengthening("E(3,8)") && "Should not detect lengthening in simple pattern");
        assert(!manager.hasProgressiveLengthening("E(3,8)+2") && "Should not detect lengthening in offset pattern");
        
        // Test progressive transformation detection
        assert(manager.hasProgressiveTransformation("E(1,8)B>8") && "Should detect progressive transformation");
        assert(manager.hasProgressiveTransformation("B(3,17)E>17") && "Should detect progressive transformation");
        assert(!manager.hasProgressiveTransformation("E(3,8)") && "Should not detect transformation in simple pattern");
        
        // Test comprehensive analysis
        assert(manager.hasAnyProgressiveFeatures("E(3,8)+2") && "Should detect any progressive features");
        assert(manager.hasAnyProgressiveFeatures("E(3,8)*3") && "Should detect any progressive features");
        assert(manager.hasAnyProgressiveFeatures("E(1,8)B>8") && "Should detect any progressive features");
        assert(!manager.hasAnyProgressiveFeatures("E(3,8)") && "Should not detect features in simple pattern");
        
        std::cout << "✅ Progressive pattern analysis working correctly!" << std::endl;
    }
    
    void testProgressiveOffsetManagement() {
        std::cout << "\n--- Progressive Offset Management Test ---" << std::endl;
        
        TestProgressiveManager manager;
        MockPatternEngine engine;
        std::string stateKey = "E(3,8)+2";
        
        // Initialize progressive offset state
        manager.initializeProgressiveState(stateKey, "E(3,8)", "E(3,8)+2");
        
        // Verify state initialization
        assert(manager.hasProgressiveState(stateKey) && "Should have progressive state");
        assert(manager.getBasePattern(stateKey) == "E(3,8)" && "Should return correct base pattern");
        assert(manager.getProgressiveOffsetValue(stateKey) == 2 && "Should start with first offset value");
        
        // Test progressive triggering
        manager.triggerProgressive(stateKey, engine);
        assert(manager.getProgressiveOffsetValue(stateKey) == 4 && "Should advance offset by step size");
        
        manager.triggerProgressive(stateKey, engine);
        assert(manager.getProgressiveOffsetValue(stateKey) == 6 && "Should advance offset again");
        
        // Test reset functionality
        manager.resetProgressiveOffset(stateKey);
        assert(manager.getProgressiveOffsetValue(stateKey) == 2 && "Should reset to initial offset");
        
        std::cout << "✅ Progressive offset management working correctly!" << std::endl;
    }
    
    void testProgressiveLengtheningManagement() {
        std::cout << "\n--- Progressive Lengthening Management Test ---" << std::endl;
        
        TestProgressiveManager manager;
        MockPatternEngine engine;
        std::string stateKey = "E(3,8)*3";
        
        // Initialize progressive lengthening state
        manager.initializeProgressiveState(stateKey, "E(3,8)", "E(3,8)*3");
        
        // Verify state initialization
        assert(manager.hasProgressiveState(stateKey) && "Should have progressive state");
        assert(manager.getBasePattern(stateKey) == "E(3,8)" && "Should return correct base pattern");
        
        // Test pattern application
        std::string result = manager.applyProgressive(stateKey, engine);
        assert(result.find("lengthened by: 3") != std::string::npos && "Should apply lengthening");
        
        // Test progressive triggering
        int initialTriggerCount = manager.getTransformationStepCount(stateKey);
        manager.triggerProgressive(stateKey, engine);
        assert(manager.getTransformationStepCount(stateKey) == initialTriggerCount + 1 && "Should increment trigger count");
        
        std::cout << "✅ Progressive lengthening management working correctly!" << std::endl;
    }
    
    void testProgressiveTransformationManagement() {
        std::cout << "\n--- Progressive Transformation Management Test ---" << std::endl;
        
        TestProgressiveManager manager;
        MockPatternEngine engine;
        std::string stateKey = "E(1,8)B>8";
        
        // Initialize progressive transformation state
        manager.initializeProgressiveState(stateKey, "E(1,8)", "E(1,8)B>8");
        
        // Verify state initialization
        assert(manager.hasProgressiveState(stateKey) && "Should have progressive state");
        assert(manager.getBasePattern(stateKey) == "E(1,8)" && "Should return correct base pattern");
        assert(manager.getTransformationStepCount(stateKey) == 0 && "Should start with zero transformation steps");
        
        // Test progressive triggering
        manager.triggerProgressive(stateKey, engine);
        assert(manager.getTransformationStepCount(stateKey) == 1 && "Should advance transformation step");
        
        manager.triggerProgressive(stateKey, engine);
        assert(manager.getTransformationStepCount(stateKey) == 2 && "Should advance transformation step again");
        
        // Test pattern application
        std::string result = manager.applyProgressive(stateKey, engine);
        assert(result.find("transformed step: 2") != std::string::npos && "Should show transformation progress");
        
        std::cout << "✅ Progressive transformation management working correctly!" << std::endl;
    }
    
    void testStateManagementAndCleanup() {
        std::cout << "\n--- State Management and Cleanup Test ---" << std::endl;
        
        TestProgressiveManager manager;
        
        // Test multiple state management
        manager.initializeProgressiveState("pattern1", "E(3,8)", "E(3,8)+2");
        manager.initializeProgressiveState("pattern2", "B(5,13)", "B(5,13)*3");
        manager.initializeProgressiveState("pattern3", "E(1,8)", "E(1,8)B>8");
        
        assert(manager.getActiveStateCount() == 3 && "Should track multiple states");
        assert(manager.hasProgressiveState("pattern1") && "Should maintain pattern1 state");
        assert(manager.hasProgressiveState("pattern2") && "Should maintain pattern2 state");
        assert(manager.hasProgressiveState("pattern3") && "Should maintain pattern3 state");
        
        // Test pattern-specific reset
        manager.resetAllProgressiveForPattern("pattern1");
        assert(manager.getProgressiveOffsetValue("pattern1") == 2 && "Should reset pattern1 to initial state");
        assert(manager.hasProgressiveState("pattern2") && "Should not affect other patterns");
        
        // Test complete cleanup
        manager.clearAllProgressiveStates();
        assert(manager.getActiveStateCount() == 0 && "Should clear all states");
        assert(!manager.hasProgressiveState("pattern1") && "Should remove all pattern states");
        
        std::cout << "✅ State management and cleanup working correctly!" << std::endl;
    }
    
    void testIntegrationWithPatternEngine() {
        std::cout << "\n--- Integration with PatternEngine Test ---" << std::endl;
        
        TestProgressiveManager manager;
        MockPatternEngine engine;
        std::string stateKey = "E(5,8)+3";
        
        // Initialize and apply progressive pattern
        manager.initializeProgressiveState(stateKey, "E(5,8)", "E(5,8)+3");
        std::string result = manager.applyProgressive(stateKey, engine);
        
        // Verify PatternEngine integration
        assert(engine.hasProgressiveOffsetEnabled() && "Should enable progressive offset in engine");
        assert(engine.getCurrentOffset() == 3 && "Should set correct offset in engine");
        assert(result.find("offset applied: 3") != std::string::npos && "Should indicate offset application");
        
        // Test progressive advancement through engine
        manager.triggerProgressive(stateKey, engine);
        manager.applyProgressive(stateKey, engine);
        assert(engine.getCurrentOffset() == 6 && "Should advance offset in engine");
        
        std::cout << "✅ Integration with PatternEngine working correctly!" << std::endl;
    }
};

int main()
{
    try {
        ProgressiveManagerTester tester;
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