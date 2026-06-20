/*
  ==============================================================================

    AccentSequence.h
    Immutable accent sequence for robust polymetric patterns
    
    Part of the 3-canonical-source robust accent system architecture.
    Provides deterministic, pre-calculated accent lookups with O(1) performance.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cstdint>

//==============================================================================
/**
 * Immutable accent sequence for robust polymetric pattern handling
 * 
 * This class eliminates accent system fragility by pre-calculating the entire
 * accent sequence for the full LCM (Least Common Multiple) cycle of rhythm 
 * and accent patterns. Once created, the sequence is immutable and provides
 * deterministic O(1) accent lookups.
 * 
 * Key benefits:
 * - No real-time calculations during playback
 * - Perfect UI/MIDI synchronization (both use same immutable source)
 * - Handles any polymetric complexity through LCM calculation
 * - Eliminates accent swirling and timing drift issues
 * 
 * Example usage:
 * ```cpp
 * std::vector<bool> rhythm = {1,0,1,1,0,1,1,0};  // E(5,8)
 * std::vector<bool> accent = {1,0};               // {10}
 * AccentSequence sequence(rhythm, accent);
 * 
 * // O(1) lookups - no real-time calculation needed
 * bool isAccented = sequence.isAccentedAtStep(stepInSequence);
 * std::vector<bool> uiMap = sequence.getAccentMapForCycle(cycleStart);
 * ```
 */
class AccentSequence
{
public:
    //==============================================================================
    /**
     * Create immutable accent sequence from rhythm and accent patterns
     * 
     * @param rhythmPattern Binary rhythm pattern (1=onset, 0=rest)
     * @param accentPattern Binary accent pattern applied to onsets
     * 
     * The sequence length will be LCM(rhythm_steps, accent_onsets) to ensure
     * the pattern repeats correctly. All accent decisions are pre-calculated
     * at construction time.
     * 
     * @throws std::invalid_argument if patterns are invalid
     */
    AccentSequence(const std::vector<bool>& rhythmPattern, 
                   const std::vector<bool>& accentPattern);
    
    //==============================================================================
    // Core accent lookup functionality
    
    /**
     * Check if a specific step in the sequence has an accented onset
     * 
     * @param stepInSequence Step position in the full sequence (0-based)
     * @return true if step has an onset AND that onset should be accented
     * 
     * Performance: O(1) - direct table lookup, no calculations
     * Thread-safe: Yes - immutable data after construction
     */
    bool isAccentedAtStep(uint32_t stepInSequence) const;
    
    /**
     * Get accent map for one rhythm cycle starting at specified step
     * 
     * @param cycleStartStep Starting step for the rhythm cycle
     * @return Vector of accent markers for UI display (size = rhythm cycle length)
     * 
     * Used by UI to display accent markers without real-time calculation.
     * Returns accent status for each step in one complete rhythm cycle.
     */
    std::vector<bool> getAccentMapForCycle(uint32_t cycleStartStep) const;
    
    //==============================================================================
    // Sequence properties and validation
    
    /**
     * Get the full sequence length (LCM of rhythm and accent cycles)
     * 
     * @return Number of steps in complete sequence before pattern repeats
     * 
     * This is the LCM of rhythm pattern length and the number of onsets
     * needed for accent pattern completion.
     */
    uint32_t getSequenceLength() const { return fullSequenceLength; }
    
    /**
     * Get the rhythm cycle length
     * 
     * @return Number of steps in one rhythm pattern cycle
     */
    uint32_t getRhythmCycleLength() const { return rhythmCycleLength; }
    
    /**
     * Get the accent cycle length (in onsets, not steps)
     * 
     * @return Number of onsets needed to complete one accent pattern cycle
     */
    uint32_t getAccentCycleLength() const { return accentCycleLength; }
    
    /**
     * Validate that the sequence was constructed successfully
     * 
     * @return true if sequence is valid and ready for use
     */
    bool isValid() const { return !accentTable.empty() && fullSequenceLength > 0; }
    
    //==============================================================================
    // Advanced functionality for debugging and analysis
    
    /**
     * Check if a step has a rhythm onset (regardless of accent)
     * 
     * @param stepInSequence Step position in the full sequence
     * @return true if step has a rhythm onset
     */
    bool hasOnsetAtStep(uint32_t stepInSequence) const;
    
    /**
     * Get the onset number for a step (for debugging)
     * 
     * @param stepInSequence Step position in the full sequence  
     * @return Onset number (0-based) or -1 if step has no onset
     */
    int getOnsetNumberAtStep(uint32_t stepInSequence) const;
    
    /**
     * Get debug information about the sequence
     * 
     * @return String containing sequence statistics and validation info
     */
    juce::String getDebugInfo() const;

private:
    //==============================================================================
    // Immutable sequence data
    
    std::vector<bool> accentTable;        // Pre-calculated accent decisions for full sequence
    std::vector<bool> rhythmPattern;      // Original rhythm pattern for cycle mapping
    std::vector<bool> accentPattern;      // Original accent pattern for validation
    
    uint32_t rhythmCycleLength;           // Steps per rhythm cycle
    uint32_t accentCycleLength;           // Onsets per accent cycle  
    uint32_t fullSequenceLength;          // LCM(rhythm_steps, accent_onsets) in steps
    
    //==============================================================================
    // Construction helpers
    
    /**
     * Calculate LCM of two positive integers
     */
    static uint32_t calculateLCM(uint32_t a, uint32_t b);
    
    /**
     * Calculate GCD of two positive integers (for LCM calculation)
     */
    static uint32_t calculateGCD(uint32_t a, uint32_t b);
    
    /**
     * Generate the full accent sequence table
     */
    void generateAccentSequence();
    
    /**
     * Validate input patterns
     */
    void validatePatterns(const std::vector<bool>& rhythmPattern,
                         const std::vector<bool>& accentPattern);
    
    /**
     * Count onsets in rhythm pattern
     */
    uint32_t countOnsets(const std::vector<bool>& pattern) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AccentSequence)
};