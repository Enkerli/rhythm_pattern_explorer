//==============================================================================
// AccentPreservationDebugTests.cpp  
// Debug test for the exact user scenario: {1010}E(4,17)E>17 with accents on steps 0,7,14
// Reproduces the accent preservation failure
//==============================================================================

#include <iostream>
#include <cassert>
#include <vector>

// Mock pattern engine
class MockPatternEngine {
private:
    std::vector<bool> pattern;
public:
    void setPattern(const std::vector<bool>& p) { pattern = p; }
    const std::vector<bool>& getCurrentPattern() const { return pattern; }
};

// Mock audio processor that reproduces the exact bug
class MockAudioProcessor {
private:
    MockPatternEngine patternEngine;
    std::vector<bool> currentAccentPattern;
    bool hasAccentPattern = false;
    bool patternManuallyModified = false;
    int uiAccentOffset = 0; // This might be the problem
    
public:
    MockPatternEngine& getPatternEngine() { return patternEngine; }
    
    // Set up the exact user scenario
    void setupUserScenario() {
        // Pattern: E(4,17) = 4 onsets distributed across 17 steps
        // USER REPORTED: accents appear at steps 0, 7, 14 with {1010}
        // This means onsets must be at steps where accents can appear
        // If {1010} puts accents on onsets 0 and 2, and we see accents at steps 0,7,14
        // Then the onsets must be at: 0,?,7,?,14 (where ? are unaccented onsets)
        // Let's use: onsets at steps 0, 4, 7, 14 (to match user's observed accent positions)
        std::vector<bool> pattern(17, false);
        pattern[0] = true;  // Onset 0 - should be accented by {1010}
        pattern[4] = true;  // Onset 1 - should be unaccented by {1010}  
        pattern[7] = true;  // Onset 2 - should be accented by {1010}
        pattern[14] = true; // Onset 3 - should be unaccented by {1010}
        
        patternEngine.setPattern(pattern);
        
        // Accent pattern: {1010} = accents on onsets 0 and 2
        currentAccentPattern = {true, false, true, false}; // {1010}
        hasAccentPattern = true;
        
        // UI accent offset = 0 (starting position)
        uiAccentOffset = 0;
        patternManuallyModified = false; // Normal mode (UPI-based)
        
        std::cout << "Setup: E(4,17) pattern with {1010} accent cycle\n";
        std::cout << "Pattern: ";
        for (bool step : pattern) std::cout << (step ? "1" : "0");
        std::cout << " (onsets at steps 0,4,7,14)\n";
        std::cout << "Accent cycle: {1010} (accents on onsets 0,2)\n";
        std::cout << "Expected visual: accents at steps 0,7 (onset 0=accented, onset 2=accented)\n";
    }
    
    // Reproduce the getCurrentAccentMap logic exactly
    std::vector<bool> getCurrentAccentMap() const {
        std::vector<bool> accentMap;
        auto currentPattern = patternEngine.getCurrentPattern();
        
        if (!hasAccentPattern || currentAccentPattern.empty()) {
            accentMap.resize(currentPattern.size(), false);
            return accentMap;
        }
        
        accentMap.resize(currentPattern.size(), false);
        
        if (patternManuallyModified) {
            // SUSPENSION MODE: Use step-based accent mapping
            std::cout << "getCurrentAccentMap: Using SUSPENSION mode (step-based)\n";
            for (int stepIndex = 0; stepIndex < static_cast<int>(currentPattern.size()); ++stepIndex) {
                if (currentPattern[stepIndex] && stepIndex < static_cast<int>(currentAccentPattern.size())) {
                    accentMap[stepIndex] = currentAccentPattern[stepIndex];
                }
            }
        } else {
            // NORMAL MODE: Use onset-based accent mapping with UI offset
            std::cout << "getCurrentAccentMap: Using NORMAL mode (onset-based)\n";
            std::cout << "  uiAccentOffset = " << uiAccentOffset << "\n";
            std::cout << "  currentAccentPattern size = " << currentAccentPattern.size() << "\n";
            
            int onsetNumber = uiAccentOffset; // Start from stable UI accent position
            
            for (int stepInCycle = 0; stepInCycle < static_cast<int>(currentPattern.size()); ++stepInCycle) {
                if (currentPattern[stepInCycle]) {
                    // This step will have an onset - will it be accented?
                    bool isAccented = shouldOnsetBeAccented(onsetNumber);
                    accentMap[stepInCycle] = isAccented;
                    std::cout << "  Step " << stepInCycle << " (onset " << onsetNumber << "): " << (isAccented ? "ACCENTED" : "unaccented") << "\n";
                    onsetNumber++;
                }
            }
        }
        
        return accentMap;
    }
    
    // Helper method: onset-based accent logic
    bool shouldOnsetBeAccented(int onsetNumber) const {
        if (!hasAccentPattern || currentAccentPattern.empty()) return false;
        
        int accentPosition = onsetNumber % static_cast<int>(currentAccentPattern.size());
        bool accented = currentAccentPattern[accentPosition];
        
        std::cout << "    shouldOnsetBeAccented(" << onsetNumber << "): position " << accentPosition << " in {1010} = " << (accented ? "true" : "false") << "\n";
        return accented;
    }
    
    // Reproduce the toggleAccentAtStep logic exactly  
    void toggleAccentAtStep(int stepIndex) {
        auto currentPattern = patternEngine.getCurrentPattern();
        if (stepIndex < 0 || stepIndex >= currentPattern.size()) return;
        if (!currentPattern[stepIndex]) return; // Can't accent a rest
        
        std::cout << "\ntoggleAccentAtStep(" << stepIndex << "):\n";
        
        // ENTER SUSPENSION MODE: Preserve current VISUAL accent state
        if (!patternManuallyModified) {
            std::cout << "  Entering suspension mode...\n";
            
            // CRITICAL: Capture current visual accent state BEFORE setting patternManuallyModified
            std::cout << "  Capturing current visual accent state:\n";
            auto currentVisualAccents = getCurrentAccentMap(); // Call while still in normal mode
            
            std::cout << "  Captured visual accents: ";
            for (bool accent : currentVisualAccents) std::cout << (accent ? "1" : "0");
            std::cout << "\n";
            
            // Now enter suspension mode
            patternManuallyModified = true;
            
            // Set the captured visual state as our new accent pattern
            currentAccentPattern = currentVisualAccents;
            hasAccentPattern = true; // We now have a captured accent pattern
            
            std::cout << "  New currentAccentPattern: ";
            for (bool accent : currentAccentPattern) std::cout << (accent ? "1" : "0");
            std::cout << " (from captured visual state)\n";
        }
        
        // Toggle accent at this step
        if (stepIndex < currentAccentPattern.size()) {
            bool oldValue = currentAccentPattern[stepIndex];
            currentAccentPattern[stepIndex] = !currentAccentPattern[stepIndex];
            std::cout << "  Toggled accent at step " << stepIndex << " from " << oldValue << " to " << currentAccentPattern[stepIndex] << "\n";
        }
    }
    
    // Access methods
    bool isInSuspensionMode() const { return patternManuallyModified; }
    int getUIAccentOffset() const { return uiAccentOffset; }
    void setUIAccentOffset(int offset) { uiAccentOffset = offset; }
    
    // Method to manually simulate user's reported visual state
    void simulateUserReportedState() {
        // User reported: accents on steps 0, 7, 14
        // Let's force the currentAccentPattern to produce this visual state
        
        // We need the pattern to have onsets at steps 0, 7, 14 (and maybe others)
        std::vector<bool> pattern(17, false);
        pattern[0] = true;   // Onset at step 0 (will be accented)
        pattern[7] = true;   // Onset at step 7 (will be accented) 
        pattern[14] = true;  // Onset at step 14 (will be accented)
        
        // Also add the onset at step 4 to match E(4,17)
        pattern[4] = true;   // Onset at step 4 (will not be accented)
        
        patternEngine.setPattern(pattern);
        
        // Set accent pattern to match user's reported visual state
        // Steps 0,7,14 should be accented, step 4 should not
        currentAccentPattern.resize(17, false);
        currentAccentPattern[0] = true;   // Accent at step 0
        currentAccentPattern[7] = true;   // Accent at step 7
        currentAccentPattern[14] = true;  // Accent at step 14
        // Step 4 remains false (no accent)
        
        hasAccentPattern = true;
        patternManuallyModified = false; // Still in normal mode
        
        std::cout << "Set up pattern to match user's reported state:\n";
        std::cout << "  Pattern: ";
        for (bool step : pattern) std::cout << (step ? "1" : "0");
        std::cout << " (onsets at steps 0,4,7,14)\n";
        std::cout << "  Forced accents at steps 0,7,14 (as user reported)\n";
    }
};

//==============================================================================
// Test Cases
//==============================================================================

void testExactUserScenario() {
    std::cout << "=== Testing Exact User Scenario ===\n";
    std::cout << "Reproducing: {1010}E(4,17)E>17 with accents on steps 0,7,14\n\n";
    
    MockAudioProcessor processor;
    processor.setupUserScenario();
    
    // SIMULATE THE USER'S REPORTED STATE: manually set accents at steps 0,7,14
    std::cout << "SIMULATING USER'S REPORTED STATE: Manually setting accents at steps 0,7,14\n";
    // We'll override the calculated accent state to match what the user reported seeing
    
    // Show initial visual state
    std::cout << "\n1. Initial visual accent state:\n";
    auto initialAccents = processor.getCurrentAccentMap();
    std::cout << "Visual accents: ";
    for (int i = 0; i < initialAccents.size(); ++i) {
        std::cout << (initialAccents[i] ? "1" : "0");
    }
    std::cout << "\n";
    
    // Count where accents appear
    std::vector<int> accentSteps;
    for (int i = 0; i < initialAccents.size(); ++i) {
        if (initialAccents[i]) {
            accentSteps.push_back(i);
        }
    }
    
    std::cout << "Accents appear at steps: ";
    for (int step : accentSteps) std::cout << step << " ";
    std::cout << "(user sees accents here)\n";
    
    // Now click on a step to enter suspension mode
    std::cout << "\n2. User clicks on step 0 to toggle accent:\n";
    processor.toggleAccentAtStep(0);
    
    // Check what happened to the accent pattern
    std::cout << "\n3. After entering suspension mode:\n";
    auto suspendedAccents = processor.getCurrentAccentMap();
    std::cout << "Suspended accents: ";
    for (int i = 0; i < suspendedAccents.size(); ++i) {
        std::cout << (suspendedAccents[i] ? "1" : "0");
    }
    std::cout << "\n";
    
    // Count where accents appear now
    std::vector<int> suspendedAccentSteps;
    for (int i = 0; i < suspendedAccents.size(); ++i) {
        if (suspendedAccents[i]) {
            suspendedAccentSteps.push_back(i);
        }
    }
    
    std::cout << "Accents now at steps: ";
    for (int step : suspendedAccentSteps) std::cout << step << " ";
    std::cout << "\n";
    
    // Check if the preservation worked
    bool preservedCorrectly = true;
    for (int i = 0; i < std::min(initialAccents.size(), suspendedAccents.size()); ++i) {
        if (i == 0) {
            // Step 0 should be toggled
            if (suspendedAccents[i] == initialAccents[i]) {
                std::cout << "❌ Step 0 was not toggled\n";
                preservedCorrectly = false;
            }
        } else {
            // Other steps should be preserved
            if (suspendedAccents[i] != initialAccents[i]) {
                std::cout << "❌ Step " << i << " was not preserved (was " << initialAccents[i] << ", now " << suspendedAccents[i] << ")\n";
                preservedCorrectly = false;
            }
        }
    }
    
    if (preservedCorrectly) {
        std::cout << "✅ Accent preservation worked correctly\n";
    } else {
        std::cout << "❌ ACCENT PRESERVATION FAILED\n";
        std::cout << "Expected: preserve all accents except toggle step 0\n";
        std::cout << "Got: lost some of the original accent positions\n";
    }
    
    std::cout << "\n=== End Test ===\n\n";
}

void testDifferentUIAccentOffsets() {
    std::cout << "=== Testing Different UI Accent Offsets ===\n";
    std::cout << "The problem might be related to uiAccentOffset value\n\n";
    
    for (int offset = 0; offset < 4; ++offset) {
        std::cout << "Testing with uiAccentOffset = " << offset << ":\n";
        
        MockAudioProcessor processor;
        processor.setupUserScenario();
        processor.setUIAccentOffset(offset);
        
        auto accents = processor.getCurrentAccentMap();
        std::cout << "  Visual accents: ";
        for (bool accent : accents) std::cout << (accent ? "1" : "0");
        
        std::vector<int> accentSteps;
        for (int i = 0; i < accents.size(); ++i) {
            if (accents[i]) accentSteps.push_back(i);
        }
        std::cout << " (steps: ";
        for (int step : accentSteps) std::cout << step << " ";
        std::cout << ")\n";
    }
    
    std::cout << "\n=== End Offset Test ===\n\n";
}

void testUserReportedScenario() {
    std::cout << "=== Testing User's Exact Reported Scenario ===\n";
    std::cout << "User reported: accents on steps 0,7,14 before click, only step 0 after click\n\n";
    
    MockAudioProcessor processor;
    processor.simulateUserReportedState();
    
    // Show what we set up
    std::cout << "\n1. Simulated user's visual state:\n";
    auto initialAccents = processor.getCurrentAccentMap();
    std::cout << "Visual accents: ";
    for (int i = 0; i < initialAccents.size(); ++i) {
        std::cout << (initialAccents[i] ? "1" : "0");
    }
    std::cout << "\n";
    
    std::vector<int> accentSteps;
    for (int i = 0; i < initialAccents.size(); ++i) {
        if (initialAccents[i]) accentSteps.push_back(i);
    }
    std::cout << "Accents at steps: ";
    for (int step : accentSteps) std::cout << step << " ";
    std::cout << "(should be 0,7,14 as user reported)\n";
    
    // Now click on a step to test preservation
    std::cout << "\n2. User clicks on step 7 to toggle accent:\n";
    processor.toggleAccentAtStep(7);
    
    std::cout << "\n3. After clicking step 7:\n";
    auto suspendedAccents = processor.getCurrentAccentMap();
    std::cout << "Suspended accents: ";
    for (int i = 0; i < suspendedAccents.size(); ++i) {
        std::cout << (suspendedAccents[i] ? "1" : "0");
    }
    std::cout << "\n";
    
    std::vector<int> suspendedAccentSteps;
    for (int i = 0; i < suspendedAccents.size(); ++i) {
        if (suspendedAccents[i]) suspendedAccentSteps.push_back(i);
    }
    std::cout << "Accents now at steps: ";
    for (int step : suspendedAccentSteps) std::cout << step << " ";
    std::cout << "\n";
    
    // Check what the user expected vs what they got
    std::cout << "\nUser expected: step 7 toggled off, steps 0,14 preserved\n";
    std::cout << "Expected result: accents at steps 0,14\n";
    
    bool hasStep0 = suspendedAccents.size() > 0 && suspendedAccents[0];
    bool hasStep7 = suspendedAccents.size() > 7 && suspendedAccents[7];
    bool hasStep14 = suspendedAccents.size() > 14 && suspendedAccents[14];
    
    std::cout << "Actual result: step 0=" << (hasStep0 ? "accented" : "unaccented") 
              << ", step 7=" << (hasStep7 ? "accented" : "unaccented")
              << ", step 14=" << (hasStep14 ? "accented" : "unaccented") << "\n";
    
    if (hasStep0 && !hasStep7 && hasStep14) {
        std::cout << "✅ PRESERVATION WORKED: Steps 0,14 preserved, step 7 toggled off\n";
    } else {
        std::cout << "❌ PRESERVATION FAILED: Expected accents at steps 0,14 only\n";
    }
    
    std::cout << "\n=== End User Reported Scenario Test ===\n\n";
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    std::cout << "=== Accent Preservation Debug Tests ===\n\n";
    
    try {
        testExactUserScenario();
        testUserReportedScenario();
        testDifferentUIAccentOffsets();
        
        std::cout << "🔍 DEBUG COMPLETE\n";
        std::cout << "Check the output above to understand why accent preservation is failing.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ TEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ TEST FAILED: Unknown error occurred\n";
        return 1;
    }
}