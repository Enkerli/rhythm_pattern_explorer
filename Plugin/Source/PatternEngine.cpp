/*
  ==============================================================================

    Rhythm Pattern Explorer - Pattern Engine Implementation

  ==============================================================================
*/

#include "PatternEngine.h"

//==============================================================================
PatternEngine::PatternEngine()
    : randomEngine(std::random_device{}())
{
    // Initialize with a simple pattern
    currentPattern = {true, false, false, true, false, false, true, false};
    
    DBG("PatternEngine: Initialized with default pattern");
}

PatternEngine::~PatternEngine()
{
}

//==============================================================================
void PatternEngine::generateEuclideanPattern(int onsets, int steps, int offset)
{
    if (steps <= 0 || onsets < 0 || onsets > steps)
    {
        DBG("PatternEngine: Invalid Euclidean parameters: onsets=" << onsets << ", steps=" << steps);
        return;
    }
    
    currentPattern = bjorklundAlgorithm(onsets, steps);
    
    if (offset != 0)
    {
        currentPattern = rotatePattern(currentPattern, offset);
    }
    
    DBG("PatternEngine: Generated Euclidean pattern E(" << onsets << "," << steps << "," << offset << ")");
}

void PatternEngine::generatePolygonPattern(int vertices, int steps, int offset)
{
    if (vertices <= 0 || steps <= 0)
    {
        DBG("PatternEngine: Invalid polygon parameters: vertices=" << vertices << ", steps=" << steps);
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Generate regular polygon pattern
    for (int i = 0; i < vertices; ++i)
    {
        int position = static_cast<int>((i * steps + offset * steps) % (vertices * steps)) / vertices;
        if (position < steps)
        {
            currentPattern[position] = true;
        }
    }
    
    DBG("PatternEngine: Generated polygon pattern P(" << vertices << "," << offset << ") with " << steps << " steps");
}

void PatternEngine::generateRandomPattern(int onsets, int steps)
{
    if (steps <= 0 || onsets < 0 || onsets > steps)
    {
        DBG("PatternEngine: Invalid random parameters: onsets=" << onsets << ", steps=" << steps);
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
    
    DBG("PatternEngine: Generated random pattern with " << actualOnsets << " onsets in " << steps << " steps");
}

void PatternEngine::generateBinaryPattern(int binaryValue, int steps)
{
    if (steps <= 0 || steps > 32)
    {
        DBG("PatternEngine: Invalid binary parameters: value=" << binaryValue << ", steps=" << steps);
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Convert using LEFT-TO-RIGHT convention (MSB first, matching webapp standard)
    for (int i = 0; i < steps; ++i)
    {
        currentPattern[i] = (binaryValue & (1 << (steps - 1 - i))) != 0;
    }
    
    DBG("PatternEngine: Generated binary pattern " << binaryValue << " with " << steps << " steps");
}

//==============================================================================
void PatternEngine::setPattern(const std::vector<bool>& pattern)
{
    currentPattern = pattern;
    DBG("PatternEngine: Set custom pattern with " << pattern.size() << " steps");
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
    
    // Convert pattern to decimal (left-to-right, MSB first)
    int decimal = 0;
    int stepCount = static_cast<int>(currentPattern.size());
    
    for (int i = 0; i < stepCount; ++i)
    {
        if (currentPattern[i])
        {
            decimal |= (1 << (stepCount - 1 - i));
        }
    }
    
    return "0x" + juce::String::toHexString(decimal).toUpperCase();
}

juce::String PatternEngine::getOctalString() const
{
    if (currentPattern.empty())
        return "o0";
    
    // Convert pattern to decimal (left-to-right, MSB first)
    int decimal = 0;
    int stepCount = static_cast<int>(currentPattern.size());
    
    for (int i = 0; i < stepCount; ++i)
    {
        if (currentPattern[i])
        {
            decimal |= (1 << (stepCount - 1 - i));
        }
    }
    
    // Convert to octal string
    juce::String octal;
    if (decimal == 0)
    {
        octal = "0";
    }
    else
    {
        while (decimal > 0)
        {
            octal = juce::String(decimal % 8) + octal;
            decimal /= 8;
        }
    }
    
    return "o" + octal; // Prefix with 'o' for octal notation
}

juce::String PatternEngine::getDecimalString() const
{
    if (currentPattern.empty())
        return "d0";
    
    // Convert pattern to decimal (left-to-right, MSB first)
    int decimal = 0;
    int stepCount = static_cast<int>(currentPattern.size());
    
    for (int i = 0; i < stepCount; ++i)
    {
        if (currentPattern[i])
        {
            decimal |= (1 << (stepCount - 1 - i));
        }
    }
    
    return "d" + juce::String(decimal); // Prefix with 'd' for decimal notation
}

//==============================================================================
std::vector<bool> PatternEngine::bjorklundAlgorithm(int onsets, int steps)
{
    if (onsets == 0)
    {
        return std::vector<bool>(steps, false);
    }
    
    if (onsets >= steps)
    {
        return std::vector<bool>(steps, true);
    }
    
    // Bjorklund's algorithm implementation
    std::vector<std::vector<bool>> groups;
    
    // Initialize with individual onsets and rests
    for (int i = 0; i < onsets; ++i)
    {
        groups.push_back({true});
    }
    for (int i = 0; i < steps - onsets; ++i)
    {
        groups.push_back({false});
    }
    
    // Apply Bjorklund pairing algorithm
    while (groups.size() > 2)
    {
        int pairCount = std::min(static_cast<int>(groups.size()) / 2, 
                                std::min(onsets, steps - onsets));
        
        std::vector<std::vector<bool>> newGroups;
        
        // Pair groups from opposite ends
        for (int i = 0; i < pairCount; ++i)
        {
            std::vector<bool> paired;
            paired.insert(paired.end(), groups[i].begin(), groups[i].end());
            paired.insert(paired.end(), groups[groups.size() - 1 - i].begin(), 
                         groups[groups.size() - 1 - i].end());
            newGroups.push_back(paired);
        }
        
        // Add remaining unpaired groups
        for (int i = pairCount; i < groups.size() - pairCount; ++i)
        {
            newGroups.push_back(groups[i]);
        }
        
        groups = newGroups;
        
        if (groups.size() <= 2)
            break;
    }
    
    // Flatten result
    std::vector<bool> result;
    for (const auto& group : groups)
    {
        result.insert(result.end(), group.begin(), group.end());
    }
    
    return result;
}

std::vector<bool> PatternEngine::rotatePattern(const std::vector<bool>& pattern, int offset)
{
    if (pattern.empty())
        return pattern;
    
    std::vector<bool> rotated(pattern.size());
    int size = static_cast<int>(pattern.size());
    
    for (int i = 0; i < size; ++i)
    {
        int newIndex = (i + offset) % size;
        if (newIndex < 0)
            newIndex += size;
        rotated[newIndex] = pattern[i];
    }
    
    return rotated;
}

int PatternEngine::bellCurveOnsetCount(int steps)
{
    // Generate bell curve distributed onset count (avoids extremes)
    std::normal_distribution<double> distribution(steps / 2.0, (steps - 2) / 6.0);
    
    int onsets = static_cast<int>(std::round(distribution(randomEngine)));
    
    // Clamp to valid range [1, steps-1] to avoid empty or full patterns
    onsets = juce::jmax(1, juce::jmin(steps - 1, onsets));
    
    return onsets;
}
