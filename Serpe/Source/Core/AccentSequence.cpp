/*
  ==============================================================================

    AccentSequence.cpp
    Immutable accent sequence for robust polymetric patterns

  ==============================================================================
*/

#include "AccentSequence.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>

//==============================================================================
AccentSequence::AccentSequence(const std::vector<bool>& rhythmPattern, 
                               const std::vector<bool>& accentPattern)
    : rhythmPattern(rhythmPattern)
    , accentPattern(accentPattern)
    , rhythmCycleLength(0)
    , accentCycleLength(0)
    , fullSequenceLength(0)
{
    // Validate input patterns
    validatePatterns(rhythmPattern, accentPattern);
    
    // Store pattern lengths
    rhythmCycleLength = static_cast<uint32_t>(rhythmPattern.size());
    accentCycleLength = static_cast<uint32_t>(accentPattern.size());
    
    // Generate the immutable accent sequence
    generateAccentSequence();
}

//==============================================================================
bool AccentSequence::isAccentedAtStep(uint32_t stepInSequence) const
{
    if (!isValid() || accentTable.empty())
        return false;
        
    // Use modulo to handle steps beyond full sequence length
    uint32_t normalizedStep = stepInSequence % fullSequenceLength;
    
    return accentTable[normalizedStep];
}

std::vector<bool> AccentSequence::getAccentMapForCycle(uint32_t cycleStartStep) const
{
    std::vector<bool> cycleMap(rhythmCycleLength, false);
    
    if (!isValid())
        return cycleMap;
    
    // Fill one rhythm cycle worth of accent markers
    for (uint32_t i = 0; i < rhythmCycleLength; i++)
    {
        uint32_t sequenceStep = (cycleStartStep + i) % fullSequenceLength;
        cycleMap[i] = accentTable[sequenceStep];
    }
    
    return cycleMap;
}

//==============================================================================
bool AccentSequence::hasOnsetAtStep(uint32_t stepInSequence) const
{
    if (rhythmPattern.empty())
        return false;
        
    uint32_t rhythmStep = stepInSequence % rhythmCycleLength;
    return rhythmPattern[rhythmStep];
}

int AccentSequence::getOnsetNumberAtStep(uint32_t stepInSequence) const
{
    if (!hasOnsetAtStep(stepInSequence))
        return -1;
    
    // Count onsets up to this step in the full sequence
    int onsetCount = 0;
    uint32_t normalizedStep = stepInSequence % fullSequenceLength;
    
    for (uint32_t step = 0; step <= normalizedStep; step++)
    {
        if (hasOnsetAtStep(step))
        {
            if (step == normalizedStep)
                return onsetCount;
            onsetCount++;
        }
    }
    
    return -1; // Should never reach here
}

juce::String AccentSequence::getDebugInfo() const
{
    std::stringstream ss;
    
    ss << "AccentSequence Debug Info:\n";
    ss << "  Rhythm Length: " << rhythmCycleLength << " steps\n";
    ss << "  Accent Length: " << accentCycleLength << " onsets\n";
    ss << "  Full Sequence: " << fullSequenceLength << " steps\n";
    ss << "  Valid: " << (isValid() ? "Yes" : "No") << "\n";
    
    if (isValid())
    {
        uint32_t onsetCount = countOnsets(rhythmPattern);
        ss << "  Onsets per rhythm cycle: " << onsetCount << "\n";
        
        // Show first cycle of the sequence
        ss << "  First cycle pattern: ";
        for (uint32_t i = 0; i < std::min(fullSequenceLength, 32u); i++)
        {
            if (hasOnsetAtStep(i))
                ss << (accentTable[i] ? "X" : "o");
            else
                ss << ".";
        }
        if (fullSequenceLength > 32)
            ss << "...";
        ss << "\n";
        ss << "  Legend: X=accented onset, o=unaccented onset, .=rest\n";
    }
    
    return juce::String(ss.str());
}

//==============================================================================
// Private implementation
//==============================================================================

uint32_t AccentSequence::calculateLCM(uint32_t a, uint32_t b)
{
    if (a == 0 || b == 0)
        return 0;
        
    return (a * b) / calculateGCD(a, b);
}

uint32_t AccentSequence::calculateGCD(uint32_t a, uint32_t b)
{
    while (b != 0)
    {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void AccentSequence::generateAccentSequence()
{
    // Handle empty accent pattern case
    if (accentPattern.empty())
    {
        fullSequenceLength = rhythmCycleLength;
        accentTable.resize(fullSequenceLength, false);
        return;
    }
    
    // Handle pattern with no onsets case
    uint32_t onsetsPerRhythmCycle = countOnsets(rhythmPattern);
    if (onsetsPerRhythmCycle == 0)
    {
        fullSequenceLength = rhythmCycleLength;
        accentTable.resize(fullSequenceLength, false);
        return;
    }
    
    // Calculate how many rhythm cycles needed for accent pattern to complete
    // This is LCM(onsets_per_rhythm_cycle, accent_pattern_length)
    uint32_t lcmOnsets = calculateLCM(onsetsPerRhythmCycle, accentCycleLength);
    uint32_t rhythmCyclesNeeded = lcmOnsets / onsetsPerRhythmCycle;
    
    fullSequenceLength = rhythmCyclesNeeded * rhythmCycleLength;
    accentTable.resize(fullSequenceLength, false);
    
    // Generate accent sequence by walking through all steps
    uint32_t globalOnsetCount = 0;
    
    for (uint32_t step = 0; step < fullSequenceLength; step++)
    {
        uint32_t rhythmStep = step % rhythmCycleLength;
        
        if (rhythmPattern[rhythmStep])
        {
            // This step has an onset - check if it should be accented
            uint32_t accentPosition = globalOnsetCount % accentCycleLength;
            accentTable[step] = accentPattern[accentPosition];
            globalOnsetCount++;
        }
        // else: step has no onset, accentTable[step] remains false
    }
}

void AccentSequence::validatePatterns(const std::vector<bool>& rhythmPattern,
                                     const std::vector<bool>& accentPattern)
{
    // Rhythm pattern must not be empty
    if (rhythmPattern.empty())
        throw std::invalid_argument("Rhythm pattern cannot be empty");
    
    // Rhythm pattern must have at least one step
    if (rhythmPattern.size() > 10000) // Reasonable upper limit
        throw std::invalid_argument("Rhythm pattern too large (max 10000 steps)");
    
    // Accent pattern can be empty (means no accents)
    if (accentPattern.size() > 1000) // Reasonable upper limit
        throw std::invalid_argument("Accent pattern too large (max 1000 elements)");
    
    // If accent pattern exists but rhythm has no onsets, that's allowed
    // (will result in no accents, which is valid)
}

uint32_t AccentSequence::countOnsets(const std::vector<bool>& pattern) const
{
    return static_cast<uint32_t>(
        std::count(pattern.begin(), pattern.end(), true)
    );
}