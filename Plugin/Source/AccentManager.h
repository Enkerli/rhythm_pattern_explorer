/*
  ==============================================================================

    AccentManager.h
    Manages accent pattern functionality for Rhythm Pattern Explorer
    
    Encapsulates all accent-related state and logic while preserving the exact
    behavior of the original simple implementation.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/**
 * Manages accent patterns using curly bracket notation (e.g., "{100}E(3,8)")
 * 
 * CRITICAL: This encapsulation preserves the exact logic flow from the original
 * working implementation. Any changes to this logic must be validated against
 * the accent system tests.
 * 
 * Features:
 * - Accent pattern parsing with curly bracket notation
 * - Polyrhythmic accent cycling when accent and rhythm patterns differ in length
 * - Stable UI accent display (updates only at cycle boundaries)
 * - Global accent position tracking for MIDI output
 * - Integration with PatternEngine for accent visualization
 */
class AccentManager
{
public:
    AccentManager() = default;
    ~AccentManager() = default;
    
    //==============================================================================
    // Accent Pattern Management
    
    /**
     * Parse and initialize accent pattern from UPI string
     * @param upiPattern Full UPI pattern string (may contain {accent}pattern notation)
     * @return True if accent pattern was found and parsed successfully
     */
    bool parseAccentPattern(const juce::String& upiPattern);
    
    /**
     * Check if we currently have an active accent pattern
     */
    bool hasAccentPattern() const { return hasActiveAccentPattern; }
    
    /**
     * Get the current accent pattern as boolean array
     */
    const std::vector<bool>& getCurrentAccentPattern() const { return currentAccentPattern; }
    
    /**
     * Get the current accent pattern name for UI display
     */
    const juce::String& getCurrentAccentPatternName() const { return currentAccentPatternName; }
    
    /**
     * Clear current accent pattern and reset state
     */
    void clearAccentPattern();
    
    //==============================================================================
    // Accent Position Management
    
    /**
     * Get current global accent position (for MIDI output)
     */
    int getGlobalAccentPosition() const { return globalAccentPosition; }
    
    /**
     * Get stable UI accent offset (updates only at cycle boundaries)
     */
    int getUIAccentOffset() const { return uiAccentOffset; }
    
    /**
     * Advance global accent position (called on each pattern onset)
     */
    void advanceGlobalAccentPosition();
    
    /**
     * Update UI accent offset at cycle boundary
     * @param currentStep Current step in pattern for cycle boundary detection
     * @param patternSize Size of current rhythm pattern
     * @param onsetsPerCycle Number of onsets in current pattern cycle
     */
    void updateUIAccentOffsetAtCycleBoundary(int currentStep, int patternSize, int onsetsPerCycle);
    
    /**
     * Reset accent positions (called on manual triggers)
     */
    void resetAccentPositions();
    
    //==============================================================================
    // Accent Application
    
    /**
     * Check if a specific onset should be accented based on current position
     * @param onsetIndex Index of onset within current pattern
     * @return True if onset should be accented
     */
    bool shouldOnsetBeAccented(int onsetIndex) const;
    
    /**
     * Get accent map for entire current pattern cycle
     * @param rhythmPattern The rhythm pattern showing which steps contain onsets
     * @return Vector indicating which steps are accented in current cycle
     */
    std::vector<bool> getCurrentAccentMap(const std::vector<bool>& rhythmPattern) const;
    
    /**
     * Get accent position for current pattern cycle (for UI display)
     * @param currentStep Current step in pattern
     * @param patternSize Size of rhythm pattern  
     * @param onsetsPerCycle Number of onsets in current pattern cycle
     * @return Accent offset for current cycle
     */
    int getCurrentCycleAccentStart(int currentStep, int patternSize, int onsetsPerCycle) const;
    
    //==============================================================================
    // State Notifications
    
    /**
     * Check if pattern change notification should be sent to UI
     */
    bool shouldUpdateAccentDisplay() const { return patternChanged; }
    
    /**
     * Clear accent display update notification
     */
    void clearAccentDisplayUpdate() { patternChanged = false; }
    
    /**
     * Mark pattern as changed (triggers UI update)
     */
    void markPatternChanged() { patternChanged = true; }
    
    //==============================================================================
    // Debug Information
    
    /**
     * Get debug info about current accent state
     */
    juce::String getDebugInfo() const;

private:
    //==============================================================================
    // Accent Pattern State
    
    bool hasActiveAccentPattern = false;
    std::vector<bool> currentAccentPattern;
    juce::String currentAccentPatternName;
    
    // Position tracking
    int globalAccentPosition = 0;     // Global accent position counter (persists across pattern cycles)
    int uiAccentOffset = 0;           // Stable accent offset for UI display (updates only at cycle boundaries)
    
    // Change notifications
    bool patternChanged = false;      // UI update notification flag
    
    //==============================================================================
    // Helper Methods
    
    /**
     * Extract accent pattern from curly bracket notation
     * @param upiPattern Full UPI pattern string
     * @param accentPart Output: extracted accent pattern string
     * @param mainPart Output: main pattern without accent notation
     * @return True if accent pattern was found
     */
    bool extractAccentNotation(const juce::String& upiPattern, juce::String& accentPart, juce::String& mainPart);
    
    /**
     * Parse accent pattern string into boolean array
     * @param accentPatternStr Accent pattern string (e.g., "100", "E(2,5)", "P(3,0)")
     * @return True if parsing was successful
     */
    bool parseAccentPatternString(const juce::String& accentPatternStr);
    
    /**
     * Generate accent pattern from algorithmic notation (E, P, B, W, D patterns)
     * @param patternType Pattern type ('E', 'P', 'B', 'W', 'D')
     * @param onsets Number of onsets
     * @param steps Number of steps
     * @return Generated accent pattern
     */
    std::vector<bool> generateAlgorithmicAccentPattern(char patternType, int onsets, int steps);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AccentManager)
};