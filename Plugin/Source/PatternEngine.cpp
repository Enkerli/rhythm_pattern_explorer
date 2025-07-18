/*
  ==============================================================================

    Rhythm Pattern Explorer - Pattern Engine Implementation

  ==============================================================================
*/

#include "PatternEngine.h"
#include "UPIParser.h"

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
    
    currentPattern = UPIParser::bjorklundAlgorithm(onsets, steps);
    
    if (offset != 0)
    {
        currentPattern = UPIParser::rotatePattern(currentPattern, offset);
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
PatternEngine::BalanceAnalysis PatternEngine::analyzeBalance() const
{
//    BalanceAnalysis analysis;
//    
//    if (currentPattern.empty())
//    {
//        return analysis;
//    }
//    
//    // Calculate balance using Milne's perfect balance theory
//    // Sum of onset vectors in complex plane
//    double realSum = 0.0;
//    double imagSum = 0.0;
//    int onsetCount = 0;
//    
//    for (int i = 0; i < currentPattern.size(); ++i)
//    {
//        if (currentPattern[i])
//        {
//            double angle = 2.0 * juce::MathConstants<double>::pi * i / currentPattern.size();
//            realSum += std::cos(angle);
//            imagSum += std::sin(angle);
//            onsetCount++;
//        }
//    }
//    
//    // Calculate magnitude
//    analysis.magnitude = std::sqrt(realSum * realSum + imagSum * imagSum);
//    
//    if (onsetCount > 0)
//    {
//        analysis.normalizedMagnitude = analysis.magnitude / onsetCount;
//    }
//    
//    // Determine if perfectly balanced (magnitude very close to 0)
//    analysis.isPerfectlyBalanced = analysis.magnitude < 0.001;
//    
//    // Balance rating based on normalized magnitude
//    if (analysis.isPerfectlyBalanced)
//        analysis.balanceRating = "Perfect";
//    else if (analysis.normalizedMagnitude < 0.1)
//        analysis.balanceRating = "Excellent";
//    else if (analysis.normalizedMagnitude < 0.3)
//        analysis.balanceRating = "Good";
//    else if (analysis.normalizedMagnitude < 0.6)
//        analysis.balanceRating = "Fair";
//    else
//        analysis.balanceRating = "Poor";
//    
//    return analysis;
    
    // TODO: Implement balance analysis
    BalanceAnalysis analysis;
    return analysis;
}
double PatternEngine::calculateCenterOfGravityAngle() const
{
//    if (currentPattern.empty())
//    {
//        return 0.0;
//    }
//    
//    double realSum = 0.0;
//    double imagSum = 0.0;
//    int onsetCount = 0;
//    
//    for (int i = 0; i < currentPattern.size(); ++i)
//    {
//        if (currentPattern[i])
//        {
//            double angle = 2.0 * juce::MathConstants<double>::pi * i / currentPattern.size();
//            realSum += std::cos(angle);
//            imagSum += std::sin(angle);
//            onsetCount++;
//        }
//    }
//    
//    if (onsetCount == 0)
//    {
//        return 0.0;
//    }
//    
//    // Normalize by onset count to get center of gravity
//    realSum /= onsetCount;
//    imagSum /= onsetCount;
//    
//    // Calculate angle in degrees
//    double angleRadians = std::atan2(imagSum, realSum);
//    double angleDegrees = angleRadians * 180.0 / juce::MathConstants<double>::pi;
//    
//    // Normalize to 0-360 range
//    if (angleDegrees < 0)
//        angleDegrees += 360.0;
//    
//    return angleDegrees;
    
    // TODO: Implement center of gravity calculation
    return 0.0;
}
//==============================================================================
juce::String PatternEngine::getPatternDescription() const
{
    int onsets = getOnsetCount();
    int steps = getStepCount();
    
    juce::String description = "Pattern: " + juce::String(onsets) + "/" + juce::String(steps);
    
//    auto balance = analyzeBalance();
//    description += " | Balance: " + balance.balanceRating;
    
    return description;
}

juce::String PatternEngine::getBinaryString() const
{
    juce::String binary;
    for (bool step : currentPattern)
    {
        binary += step ? "1" : "0";
    }
    return binary;
}

juce::String PatternEngine::getHexString() const
{
    if (currentPattern.empty())
        return "0x0";
    
    // Convert pattern to hex using strict left-to-right notation
    // Process in 4-bit groups from left to right, where 1000 = 0x1, 0100 = 0x2, etc.
    juce::String hex;
    int stepCount = static_cast<int>(currentPattern.size());
    
    // Process pattern in 4-bit groups from left to right
    for (int groupStart = 0; groupStart < stepCount; groupStart += 4)
    {
        int nibbleValue = 0;
        
        // Process 4 bits in this group (or fewer if at the end)
        for (int bitInGroup = 0; bitInGroup < 4 && (groupStart + bitInGroup) < stepCount; ++bitInGroup)
        {
            if (currentPattern[groupStart + bitInGroup])
            {
                // Left-to-right: first bit in group is LSB of this nibble
                nibbleValue |= (1 << bitInGroup);
            }
        }
        
        hex += juce::String::toHexString(nibbleValue).toUpperCase();
    }
    
    return "0x" + hex;
}

juce::String PatternEngine::getOctalString() const
{
    if (currentPattern.empty())
        return "o0";
    
    // Convert pattern to octal using strict left-to-right notation
    // Process in 3-bit groups from left to right
    juce::String octal;
    int stepCount = static_cast<int>(currentPattern.size());
    
    // Process pattern in 3-bit groups from left to right
    for (int groupStart = 0; groupStart < stepCount; groupStart += 3)
    {
        int octalDigit = 0;
        
        // Process 3 bits in this group (or fewer if at the end)
        for (int bitInGroup = 0; bitInGroup < 3 && (groupStart + bitInGroup) < stepCount; ++bitInGroup)
        {
            if (currentPattern[groupStart + bitInGroup])
            {
                // Left-to-right: first bit in group is LSB of this octal digit
                octalDigit |= (1 << bitInGroup);
            }
        }
        
        octal += juce::String(octalDigit);
    }
    
    return "o" + octal;
}

juce::String PatternEngine::getDecimalString() const
{
    if (currentPattern.empty())
        return "d0";
    
    // Convert pattern to decimal using strict left-to-right notation
    // Process the entire pattern where leftmost bit is LSB
    int decimal = 0;
    int stepCount = static_cast<int>(currentPattern.size());
    
    // Build decimal value by reading pattern left-to-right
    // where leftmost bit has lowest positional value
    for (int i = 0; i < stepCount; ++i)
    {
        if (currentPattern[i])
        {
            // Left-to-right: bit at position i contributes 2^i
            decimal |= (1 << i);
        }
    }
    
    return "d" + juce::String(decimal);
}

//==============================================================================
// Pattern utility functions moved to UPIParser for centralization

int PatternEngine::bellCurveOnsetCount(int steps)
{
    // Generate bell curve distributed onset count (avoids extremes)
    std::normal_distribution<double> distribution(steps / 2.0, (steps - 2) / 6.0);
    
    int onsets = static_cast<int>(std::round(distribution(randomEngine)));
    
    // Clamp to valid range [1, steps-1] to avoid empty or full patterns
    onsets = juce::jmax(1, juce::jmin(steps - 1, onsets));
    
    return onsets;
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
    
    // Progressive offset configured
}

void PatternEngine::triggerProgressiveOffset()
{
    if (!hasProgressiveOffset)
        return;
    
    triggerCount++;
    currentOffset = initialOffset + (triggerCount * progressiveOffset);
    
    // Progressive offset advanced
}
