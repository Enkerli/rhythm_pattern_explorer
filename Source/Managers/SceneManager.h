/*
  ==============================================================================

    SceneManager.h
    Manages scene cycling functionality for Rhythm Pattern Explorer
    
    Encapsulates all scene-related state and logic while preserving the exact
    behavior of the original simple implementation.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/**
 * Manages scene cycling for patterns with | notation (e.g., "E(3,8)|B(5,12)|R(2,7)")
 * 
 * CRITICAL: This encapsulation preserves the exact logic flow from the original
 * working implementation. Any changes to this logic must be validated against
 * the progressive regression tests.
 */
class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;
    
    //==============================================================================
    // Scene Management
    
    /**
     * Initialize scenes from a pattern string containing | separators
     * This replicates the exact logic from the original setUPIInput implementation
     */
    void initializeScenes(const juce::StringArray& scenes);
    
    /**
     * Check if we have multiple scenes (pattern contains |)
     */
    bool hasScenes() const { return scenePatterns.size() > 1; }
    
    /**
     * Advance to the next scene in the cycle
     * Replicates original advanceScene() logic exactly
     */
    void advanceScene();
    
    /**
     * Get the current scene pattern for processing
     */
    juce::String getCurrentScenePattern() const;
    
    /**
     * Get current scene index for UI display
     */
    int getCurrentSceneIndex() const { return currentSceneIndex; }
    
    /**
     * Get total number of scenes
     */
    int getSceneCount() const { return static_cast<int>(scenePatterns.size()); }
    
    /**
     * Reset all scene state
     */
    void resetScenes();
    
    //==============================================================================
    // Per-Scene Progressive State Management
    
    /**
     * Check if current scene has progressive features
     */
    bool currentSceneHasProgressiveFeatures() const;
    
    /**
     * Get progressive offset for current scene
     */
    int getCurrentSceneProgressiveOffset() const;
    
    /**
     * Get progressive lengthening for current scene  
     */
    int getCurrentSceneProgressiveLengthening() const;
    
    /**
     * Get base pattern for current scene
     */
    juce::String getCurrentSceneBasePattern() const;
    
    /**
     * Get base length pattern for current scene (for progressive lengthening)
     */
    std::vector<bool> getCurrentSceneBaseLengthPattern() const;
    
    /**
     * Set base length pattern for current scene
     */
    void setCurrentSceneBaseLengthPattern(const std::vector<bool>& pattern);
    
    //==============================================================================
    // Debug Information
    
    /**
     * Get debug info about current scene state
     */
    juce::String getDebugInfo() const;

private:
    //==============================================================================
    // Scene Data - exactly as in original implementation
    
    juce::StringArray scenePatterns;                        // List of patterns to cycle through
    int currentSceneIndex = 0;                              // Current scene position
    
    // Per-scene progressive state tracking - exactly as in original
    std::vector<int> sceneProgressiveOffsets;               // Current offset for each scene
    std::vector<int> sceneProgressiveSteps;                 // Step size for each scene
    std::vector<juce::String> sceneBasePatterns;            // Base pattern for each scene
    std::vector<int> sceneProgressiveLengthening;           // Current lengthening for each scene
    std::vector<std::vector<bool>> sceneBaseLengthPatterns; // Base patterns for lengthening
    
    //==============================================================================
    // Helper Methods
    
    /**
     * Check if a scene pattern has progressive offset (%N or +N syntax)
     */
    bool sceneHasProgressiveOffset(const juce::String& scenePattern) const;
    
    /**
     * Check if a scene pattern has progressive lengthening (*N syntax)  
     */
    bool sceneHasProgressiveLengthening(const juce::String& scenePattern) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SceneManager)
};