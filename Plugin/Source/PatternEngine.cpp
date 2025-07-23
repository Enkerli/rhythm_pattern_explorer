/*
  ==============================================================================

    Rhythm Pattern Explorer - Pattern Engine Implementation

  ==============================================================================
*/

#include "PatternEngine.h"
#include "UPIParser.h"
#include "PatternUtils.h"

//==============================================================================
PatternEngine::PatternEngine()
    : randomEngine(std::random_device{}())
{
    // Initialize with a simple pattern
    currentPattern = {true, false, false, true, false, false, true, false};
    
}

PatternEngine::~PatternEngine()
{
}

//==============================================================================
void PatternEngine::generateEuclideanPattern(int onsets, int steps, int offset)
{
    if (steps <= 0 || onsets < 0 || onsets > steps)
    {
        return;
    }
    
    currentPattern = PatternUtils::bjorklundAlgorithm(onsets, steps);
    
    if (offset != 0)
    {
        currentPattern = PatternUtils::rotatePattern(currentPattern, offset);
    }
    
}

void PatternEngine::generatePolygonPattern(int vertices, int steps, int offset)
{
    if (vertices <= 0 || steps <= 0)
    {
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Generate regular polygon pattern - vertices equally spaced around circle
    for (int i = 0; i < vertices; ++i)
    {
        // Calculate position with floating point for accuracy, then round
        double exactPos = ((double)(i * steps) / vertices) + offset;
        int position = ((int)std::round(exactPos)) % steps;
        if (position < 0) position += steps; // Handle negative modulo
        
        if (position >= 0 && position < steps)
        {
            currentPattern[position] = true;
        }
    }
    
}

void PatternEngine::generateRandomPattern(int onsets, int steps)
{
    if (steps <= 0 || onsets < 0 || onsets > steps)
    {
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Use bell curve distribution if onsets is 0 (special case)
    int actualOnsets = onsets;
    if (onsets == 0)
    {
        actualOnsets = bellCurveOnsetCount(steps);
    }
    
    // Randomly distribute onsets
    std::vector<int> positions;
    for (int i = 0; i < steps; ++i)
    {
        positions.push_back(i);
    }
    
    std::shuffle(positions.begin(), positions.end(), randomEngine);
    
    for (int i = 0; i < actualOnsets && i < positions.size(); ++i)
    {
        currentPattern[positions[i]] = true;
    }
    
}

void PatternEngine::generateBinaryPattern(int binaryValue, int steps)
{
    if (steps <= 0 || steps > 32)
    {
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Convert using LEFT-TO-RIGHT convention (MSB first, matching webapp standard)
    for (int i = 0; i < steps; ++i)
    {
        currentPattern[i] = (binaryValue & (1 << (steps - 1 - i))) != 0;
    }
    
}

//==============================================================================
void PatternEngine::setPattern(const std::vector<bool>& pattern)
{
    currentPattern = pattern;
}

int PatternEngine::getOnsetCount() const
{
    int count = 0;
    for (bool step : currentPattern)
    {
        if (step) count++;
    }
    return count;
}


//==============================================================================
juce::String PatternEngine::getPatternDescription() const
{
    int onsets = getOnsetCount();
    int steps = getStepCount();
    
    juce::String description = "Pattern: " + juce::String(onsets) + "/" + juce::String(steps);
    
    return description;
}

juce::String PatternEngine::getBinaryString() const
{
    return PatternUtils::getBinaryString(currentPattern);
}

juce::String PatternEngine::getHexString() const
{
    return PatternUtils::getHexString(currentPattern);
}

juce::String PatternEngine::getOctalString() const
{
    return PatternUtils::getOctalString(currentPattern);
}

juce::String PatternEngine::getDecimalString() const
{
    return PatternUtils::getDecimalString(currentPattern);
}

//==============================================================================
// Pattern utility functions moved to UPIParser for centralization

int PatternEngine::bellCurveOnsetCount(int steps)
{
    return PatternUtils::bellCurveOnsetCount(steps, randomEngine);
}

//==============================================================================
// Progressive Offset Support

void PatternEngine::setProgressiveOffset(bool enabled, int initial, int progressive)
{
    hasProgressiveOffset = enabled;
    initialOffset = initial;
    progressiveOffset = progressive;
    currentOffset = initial;
    triggerCount = 0;
    
}

void PatternEngine::triggerProgressiveOffset()
{
    if (!hasProgressiveOffset)
        return;
    
    triggerCount++;
    currentOffset = initialOffset + (triggerCount * progressiveOffset);
    
}
