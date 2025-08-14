/*
  ==============================================================================

    ProgressiveManager.h
    Encapsulates all progressive pattern logic for Rhythm Pattern Explorer

    Handles:
    - Progressive offsets (+N notation)
    - Progressive lengthening (*N notation) 
    - Progressive transformations (>N notation)
    - State management and cleanup
    - Integration with PatternEngine

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <map>
#include <memory>
#include <random>

// Forward declarations
class PatternEngine;

//==============================================================================
/**
 * Manages all progressive pattern functionality for the Rhythm Pattern Explorer
 * 
 * Progressive patterns modify base patterns through repeated triggers:
 * - Progressive Offset (+N): E(3,8)+2 rotates pattern by +2 each trigger
 * - Progressive Lengthening (*N): E(3,8)*3 adds 3 random steps each trigger
 * - Progressive Transformation (>N): B(1,17)B>17 evolves pattern toward target
 * 
 * Features:
 * - Per-pattern state tracking using unique keys
 * - LRU cleanup to prevent memory growth
 * - Integration with PatternEngine for offset handling
 * - Scene-aware state management
 */
class ProgressiveManager
{
public:
    //==============================================================================
    ProgressiveManager();
    ~ProgressiveManager();

    //==============================================================================
    // Progressive Pattern Analysis
    
    /**
     * Check if pattern uses any progressive notation (+N, *N, >N)
     * @param upiPattern UPI pattern string to analyze
     */
    bool hasAnyProgressiveFeatures(const juce::String& upiPattern) const;
    
    /**
     * Check if pattern uses progressive offset (+N)
     */
    bool hasProgressiveOffset(const juce::String& upiPattern) const;
    
    /**
     * Check if pattern uses progressive lengthening (*N)
     */
    bool hasProgressiveLengthening(const juce::String& upiPattern) const;
    
    /**
     * Check if pattern uses progressive transformation (>N)
     */
    bool hasProgressiveTransformation(const juce::String& upiPattern) const;

    //==============================================================================
    // Progressive State Management
    
    /**
     * Initialize progressive state for a pattern
     * @param stateKey Unique identifier for this pattern's state
     * @param basePattern Base UPI pattern without progressive notation
     * @param progressivePattern Full UPI pattern with progressive notation
     */
    void initializeProgressiveState(const juce::String& stateKey, 
                                  const juce::String& basePattern,
                                  const juce::String& progressivePattern);
    
    /**
     * Apply progressive transformation to pattern
     * @param stateKey Unique identifier for pattern state
     * @param patternEngine PatternEngine instance for offset handling
     * @return Processed pattern result
     */
    juce::String applyProgressive(const juce::String& stateKey, PatternEngine& patternEngine);
    
    /**
     * Trigger progressive advancement for pattern
     * @param stateKey Unique identifier for pattern state
     * @param patternEngine PatternEngine instance for offset updates
     */
    void triggerProgressive(const juce::String& stateKey, PatternEngine& patternEngine);

    //==============================================================================
    // Progressive Offset Management
    
    /**
     * Get current progressive offset value for pattern
     */
    int getProgressiveOffsetValue(const juce::String& stateKey) const;
    
    /**
     * Check if we have progressive state for this key
     */
    bool hasProgressiveState(const juce::String& stateKey) const;
    
    /**
     * Get base pattern for existing state
     */
    juce::String getBasePattern(const juce::String& stateKey) const;
    
    /**
     * Reset progressive offset for pattern
     */
    void resetProgressiveOffset(const juce::String& stateKey);

    //==============================================================================
    // Progressive Lengthening Management
    
    /**
     * Get current lengthened pattern
     */
    std::vector<bool> getCurrentLengthenedPattern(const juce::String& stateKey) const;
    
    /**
     * Reset progressive lengthening for pattern
     */
    void resetProgressiveLengthening(const juce::String& stateKey);

    //==============================================================================
    // Progressive Transformation Management
    
    /**
     * Get current transformation step count for UI display
     */
    int getTransformationStepCount(const juce::String& stateKey) const;
    
    /**
     * Reset progressive transformation for pattern
     */
    void resetProgressiveTransformation(const juce::String& stateKey);

    //==============================================================================
    // State Cleanup
    
    /**
     * Reset all progressive state for specific pattern
     */
    void resetAllProgressiveForPattern(const juce::String& stateKey);
    
    /**
     * Clear all progressive states (for plugin reset)
     */
    void clearAllProgressiveStates();
    
    /**
     * Clean up least recently used states if over memory limit
     */
    void cleanupLRUStates();

    //==============================================================================
    // State Serialization (Phase 4: Progressive State Persistence)
    
    /**
     * Serialize all progressive states to ValueTree
     */
    void saveProgressiveStatesToValueTree(juce::ValueTree& stateTree) const;
    
    /**
     * Restore all progressive states from ValueTree
     */
    void loadProgressiveStatesFromValueTree(const juce::ValueTree& stateTree);

    //==============================================================================
    // Debug and Diagnostics
    
    /**
     * Get count of active progressive states
     */
    int getActiveStateCount() const;
    
    /**
     * Get debug information about progressive states
     */
    juce::String getDebugInfo() const;

private:
    //==============================================================================
    // Progressive State Data
    
    struct ProgressiveState
    {
        // Pattern info
        juce::String basePattern;
        juce::String fullPattern;
        
        // Progressive Offset (+N)
        bool hasOffset = false;
        int offsetStep = 0;           // How much to advance each trigger
        int currentOffset = 0;        // Current accumulated offset
        int triggerCount = 0;         // Number of triggers for UI display
        
        // Progressive Lengthening (*N)
        bool hasLengthening = false;
        int lengtheningStep = 0;      // How many steps to add each trigger
        std::vector<bool> baseLengthPattern; // Original pattern for lengthening
        std::vector<bool> currentLengthenedPattern; // Current lengthened result
        
        // Progressive Transformation (>N)
        bool hasTransformation = false;
        juce::String transformationType; // "B", "W", "E", "D"
        int targetOnsets = 0;         // Target onset count
        int transformationStep = 0;   // Current transformation progress
        std::vector<bool> currentTransformedPattern; // Current result
        
        // LRU tracking
        int accessCount = 0;
        juce::Time lastAccessed;
    };
    
    std::map<juce::String, ProgressiveState> progressiveStates;
    static const int MAX_PROGRESSIVE_STATES = 100;  // Memory limit
    
    // Random number generation for lengthening
    std::mt19937 randomGenerator;

    //==============================================================================
    // Helper Methods
    
    /**
     * Parse progressive notation from UPI pattern
     */
    void parseProgressiveNotation(const juce::String& pattern, ProgressiveState& state);
    
    /**
     * Apply progressive offset to pattern
     */
    juce::String applyProgressiveOffset(ProgressiveState& state, PatternEngine& patternEngine);
    
    /**
     * Apply progressive lengthening to pattern
     */
    juce::String applyProgressiveLengthening(ProgressiveState& state);
    
    /**
     * Apply progressive transformation to pattern
     */
    juce::String applyProgressiveTransformation(ProgressiveState& state);
    
    /**
     * Generate bell curve random steps for lengthening
     */
    std::vector<bool> generateBellCurveRandomSteps(int numSteps);
    
    /**
     * Update LRU access tracking
     */
    void updateLRUAccess(const juce::String& stateKey);
    
    /**
     * Extract base pattern without progressive notation
     */
    juce::String extractBasePattern(const juce::String& pattern) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressiveManager)
};