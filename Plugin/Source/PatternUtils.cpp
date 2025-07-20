/*
  ==============================================================================

    PatternUtils.cpp
    Centralized Pattern Manipulation Utilities Implementation
    
    Contains all pattern transformation, generation, and utility functions
    that were previously scattered across PatternEngine and UPIParser.

  ==============================================================================
*/

#include "PatternUtils.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>

namespace PatternUtils
{
    //==============================================================================
    // Core Pattern Transformations
    //==============================================================================
    
    std::vector<bool> invertPattern(const std::vector<bool>& pattern)
    {
        std::vector<bool> result = pattern;
        for (size_t i = 0; i < result.size(); ++i)
        {
            result[i] = !result[i];
        }
        return result;
    }
    
    std::vector<bool> reversePattern(const std::vector<bool>& pattern)
    {
        std::vector<bool> result = pattern;
        std::reverse(result.begin(), result.end());
        return result;
    }
    
    std::vector<bool> complementPattern(const std::vector<bool>& pattern)
    {
        return invertPattern(pattern);
    }
    
    std::vector<bool> rotatePattern(const std::vector<bool>& pattern, int steps)
    {
        if (pattern.empty()) return pattern;
        
        std::vector<bool> result = pattern;
        int size = static_cast<int>(pattern.size());
        
        // Normalize steps to be within pattern size
        steps = steps % size;
        if (steps < 0) steps += size;
        
        std::rotate(result.begin(), result.begin() + steps, result.end());
        return result;
    }
    
    std::vector<bool> combinePatterns(const std::vector<bool>& pattern1, 
                                     const std::vector<bool>& pattern2, 
                                     bool isAddition)
    {
        if (pattern1.empty()) return pattern2;
        if (pattern2.empty()) return pattern1;
        
        // Expand both patterns to their LCM length
        int lcmSize = lcm(static_cast<int>(pattern1.size()), static_cast<int>(pattern2.size()));
        auto expanded1 = expandToLCM(pattern1, lcmSize);
        auto expanded2 = expandToLCM(pattern2, lcmSize);
        
        std::vector<bool> result(lcmSize);
        
        if (isAddition)
        {
            // Logical OR for addition
            for (int i = 0; i < lcmSize; ++i)
            {
                result[i] = expanded1[i] || expanded2[i];
            }
        }
        else
        {
            // AND-NOT for subtraction (pattern1 AND (NOT pattern2))
            for (int i = 0; i < lcmSize; ++i)
            {
                result[i] = expanded1[i] && !expanded2[i];
            }
        }
        
        return result;
    }
    
    //==============================================================================
    // Pattern Generation Core Functions
    //==============================================================================
    
    std::vector<bool> bjorklundAlgorithm(int beats, int steps)
    {
        if (beats > steps) beats = steps;
        if (beats <= 0) return std::vector<bool>(steps, false);
        if (beats == steps) return std::vector<bool>(steps, true);
        
        std::vector<bool> pattern;
        std::vector<int> counts;
        std::vector<int> remainders;
        
        int divisor = steps - beats;
        remainders.push_back(beats);
        int level = 0;
        
        do {
            counts.push_back(divisor / remainders[level]);
            remainders.push_back(divisor % remainders[level]);
            divisor = remainders[level];
            level++;
        } while (remainders[level] > 1);
        
        counts.push_back(divisor);
        
        std::function<void(int)> build = [&](int level) {
            if (level == -1) {
                pattern.push_back(false);
            } else if (level == -2) {
                pattern.push_back(true);
            } else {
                for (int i = 0; i < counts[level]; i++) {
                    build(level - 1);
                }
                if (remainders[level] != 0) {
                    build(level - 2);
                }
            }
        };
        
        build(level);
        
        // Ensure we have exactly 'steps' elements
        while (pattern.size() < static_cast<size_t>(steps)) {
            pattern.push_back(false);
        }
        pattern.resize(steps);
        
        // Rotate so first beat is at position 0
        auto firstBeatIndex = std::find(pattern.begin(), pattern.end(), true);
        if (firstBeatIndex != pattern.begin() && firstBeatIndex != pattern.end()) {
            size_t index = std::distance(pattern.begin(), firstBeatIndex);
            std::vector<bool> rotated;
            rotated.insert(rotated.end(), pattern.begin() + index, pattern.end());
            rotated.insert(rotated.end(), pattern.begin(), pattern.begin() + index);
            pattern = rotated;
        }
        
        return pattern;
    }
    
    std::vector<bool> expandToLCM(const std::vector<bool>& pattern, int targetSize)
    {
        if (pattern.empty() || targetSize <= 0) return std::vector<bool>(targetSize, false);
        
        std::vector<bool> result;
        result.reserve(targetSize);
        
        int patternSize = static_cast<int>(pattern.size());
        for (int i = 0; i < targetSize; ++i)
        {
            result.push_back(pattern[i % patternSize]);
        }
        
        return result;
    }
    
    std::vector<bool> dilatePattern(const std::vector<bool>& pattern, int newSteps)
    {
        if (pattern.empty() || newSteps <= 0) return std::vector<bool>(newSteps, false);
        
        int originalSteps = static_cast<int>(pattern.size());
        if (newSteps == originalSteps) return pattern;
        
        std::vector<bool> result(newSteps, false);
        
        // Map each onset to its new position
        for (int i = 0; i < originalSteps; ++i)
        {
            if (pattern[i])
            {
                int newIndex = static_cast<int>((static_cast<double>(i) * newSteps) / originalSteps);
                if (newIndex < newSteps)
                {
                    result[newIndex] = true;
                }
            }
        }
        
        return result;
    }
    
    std::vector<bool> concentratePattern(const std::vector<bool>& pattern, int newSteps)
    {
        if (pattern.empty() || newSteps <= 0) return std::vector<bool>(newSteps, false);
        
        int originalSteps = static_cast<int>(pattern.size());
        if (newSteps == originalSteps) return pattern;
        
        std::vector<bool> result(newSteps, false);
        
        // Map each position in the new pattern to check if any onset exists in corresponding range
        for (int i = 0; i < newSteps; ++i)
        {
            double startPos = (static_cast<double>(i) * originalSteps) / newSteps;
            double endPos = (static_cast<double>(i + 1) * originalSteps) / newSteps;
            
            // Check if any onset exists in this range
            for (int j = static_cast<int>(startPos); j < std::min(static_cast<int>(std::ceil(endPos)), originalSteps); ++j)
            {
                if (pattern[j])
                {
                    result[i] = true;
                    break;
                }
            }
        }
        
        return result;
    }
    
    //==============================================================================
    // Pattern Utility Functions  
    //==============================================================================
    
    int countOnsets(const std::vector<bool>& pattern)
    {
        return static_cast<int>(std::count(pattern.begin(), pattern.end(), true));
    }
    
    bool isValidPattern(const std::vector<bool>& pattern)
    {
        return !pattern.empty();
    }
    
    juce::String patternToBinary(const std::vector<bool>& pattern)
    {
        juce::String binary;
        for (bool step : pattern)
        {
            binary += step ? "1" : "0";
        }
        return binary;
    }
    
    //==============================================================================
    // Mathematical Utilities
    //==============================================================================
    
    int gcd(int a, int b)
    {
        return b == 0 ? a : gcd(b, a % b);
    }
    
    int lcm(int a, int b)
    {
        return (a * b) / gcd(a, b);
    }
    
    int bellCurveOnsetCount(int steps, std::mt19937& randomEngine)
    {
        // Generate bell curve distributed onset count (avoids extremes)
        std::normal_distribution<double> distribution(steps / 2.0, (steps - 2) / 6.0);
        
        int onsets = static_cast<int>(std::round(distribution(randomEngine)));
        
        // Clamp to valid range [1, steps-1] to avoid empty or full patterns
        return std::max(1, std::min(steps - 1, onsets));
    }
    
    //==============================================================================
    // Pattern Format Conversion
    //==============================================================================
    
    juce::String getBinaryString(const std::vector<bool>& pattern)
    {
        return patternToBinary(pattern);
    }
    
    juce::String getHexString(const std::vector<bool>& pattern)
    {
        if (pattern.empty())
            return "0x0";
        
        // Convert pattern to hex using strict left-to-right notation
        // Process in 4-bit groups from left to right
        juce::String hex;
        int stepCount = static_cast<int>(pattern.size());
        
        // Process pattern in 4-bit groups from left to right
        for (int groupStart = 0; groupStart < stepCount; groupStart += 4)
        {
            int nibbleValue = 0;
            
            // Process 4 bits in this group (or fewer if at the end)  
            for (int bitInGroup = 0; bitInGroup < 4 && (groupStart + bitInGroup) < stepCount; ++bitInGroup)
            {
                if (pattern[groupStart + bitInGroup])
                {
                    // Left-to-right: each bit position maps directly to bit position in nibble
                    nibbleValue |= (1 << bitInGroup);
                }
            }
            
            hex += juce::String::toHexString(nibbleValue).toUpperCase();
        }
        
        return "0x" + hex;
    }
    
    juce::String getOctalString(const std::vector<bool>& pattern)
    {
        if (pattern.empty())
            return "o0";
        
        // Convert pattern to octal using strict left-to-right notation
        // Process in 3-bit groups from left to right
        juce::String octal;
        int stepCount = static_cast<int>(pattern.size());
        
        // Process pattern in 3-bit groups from left to right
        for (int groupStart = 0; groupStart < stepCount; groupStart += 3)
        {
            int octalDigit = 0;
            
            // Process 3 bits in this group (or fewer if at the end)
            for (int bitInGroup = 0; bitInGroup < 3 && (groupStart + bitInGroup) < stepCount; ++bitInGroup)
            {
                if (pattern[groupStart + bitInGroup])
                {
                    // Left-to-right: each bit position maps directly to bit position in octal digit
                    octalDigit |= (1 << bitInGroup);
                }
            }
            
            octal += juce::String(octalDigit);
        }
        
        return "o" + octal;
    }
    
    juce::String getDecimalString(const std::vector<bool>& pattern)
    {
        if (pattern.empty())
            return "d0";
        
        // Convert pattern to decimal using strict left-to-right notation
        // Process the entire pattern where leftmost bit is LSB
        int decimal = 0;
        int stepCount = static_cast<int>(pattern.size());
        
        // Build decimal value by reading pattern left-to-right
        // where leftmost bit has lowest positional value
        for (int i = 0; i < stepCount; ++i)
        {
            if (pattern[i])
            {
                // Left-to-right: bit at position i contributes 2^i
                decimal |= (1 << i);
            }
        }
        
        return "d" + juce::String(decimal);
    }
    
    //==============================================================================
    // Accent Pattern Functions
    //==============================================================================
    
    bool hasAccentPattern(const juce::String& input)
    {
        return input.contains("{") && input.contains("}");
    }
    
    juce::String extractAccentPattern(const juce::String& input)
    {
        int startPos = input.indexOf("{");
        int endPos = input.indexOf("}");
        
        if (startPos != -1 && endPos != -1 && endPos > startPos)
        {
            return input.substring(startPos + 1, endPos);
        }
        
        return "";
    }
    
    juce::String removeAccentPattern(const juce::String& input)
    {
        int startPos = input.indexOf("{");
        int endPos = input.indexOf("}");
        
        if (startPos != -1 && endPos != -1 && endPos > startPos)
        {
            return input.substring(0, startPos) + input.substring(endPos + 1);
        }
        
        return input;
    }
    
    //==============================================================================
    // String Processing Utilities
    //==============================================================================
    
    juce::String cleanInput(const juce::String& input)
    {
        return input.trim().toLowerCase();
    }
    
    juce::StringArray tokenize(const juce::String& input, const juce::String& delimiter)
    {
        juce::StringArray tokens;
        juce::String remaining = input;
        
        int delimiterPos;
        while ((delimiterPos = remaining.indexOf(delimiter)) != -1)
        {
            tokens.add(remaining.substring(0, delimiterPos));
            remaining = remaining.substring(delimiterPos + delimiter.length());
        }
        
        if (!remaining.isEmpty())
        {
            tokens.add(remaining);
        }
        
        return tokens;
    }
    
    //==============================================================================
    // Advanced Pattern Generation
    //==============================================================================
    
    std::vector<bool> generateBarlowTransformation(const std::vector<bool>& originalPattern, 
                                                  int targetOnsets, 
                                                  bool wolrabMode)
    {
        int stepCount = static_cast<int>(originalPattern.size());
        int currentOnsets = countOnsets(originalPattern);
        
        if (currentOnsets == targetOnsets) return originalPattern;
        
        // Calculate Barlow indispensability for all positions
        std::vector<std::pair<int, double>> allPositions;
        for (int i = 0; i < stepCount; ++i)
        {
            double indispensability = calculateBarlowIndispensability(i, stepCount);
            allPositions.push_back({i, indispensability});
        }
        
        if (targetOnsets < currentOnsets)
        {
            // Dilution: remove onsets
            return Internal::diluteByBarlow(originalPattern, targetOnsets, allPositions, wolrabMode);
        }
        else
        {
            // Concentration: add onsets
            return Internal::concentrateByBarlow(originalPattern, targetOnsets, allPositions, wolrabMode);
        }
    }
    
    std::vector<bool> generateEuclideanTransformation(const std::vector<bool>& originalPattern,
                                                     int targetOnsets,
                                                     bool antiMode)
    {
        int stepCount = static_cast<int>(originalPattern.size());
        
        if (targetOnsets == 0)
        {
            return std::vector<bool>(stepCount, false);
        }
        else if (targetOnsets == stepCount)
        {
            return std::vector<bool>(stepCount, true);
        }
        else if (antiMode)
        {
            // Dilcue (Anti-Euclidean): use complement pattern
            auto euclideanPattern = bjorklundAlgorithm(stepCount - targetOnsets, stepCount);
            return invertPattern(euclideanPattern);
        }
        else
        {
            // Normal Euclidean using Bjorklund algorithm
            return bjorklundAlgorithm(targetOnsets, stepCount);
        }
    }
    
    double calculateBarlowIndispensability(int position, int stepCount)
    {
        // Authentic Barlow indispensability based on Clarence Barlow's theory
        // Uses algorithmic approach that works for ALL step counts including primes
        
        if (position == 0) {
            // Downbeat always has maximum indispensability
            return 10.0;
        }
        
        // Calculate indispensability using metric strength theory
        // This creates proper hierarchy even for prime step counts
        
        double indispensability = 0.0;
        
        // Method 1: GCD-based metric strength (works for composite numbers)
        int gcd_value = gcd(position, stepCount);
        if (gcd_value > 1) {
            // Position aligns with a metric subdivision
            indispensability = static_cast<double>(gcd_value) / stepCount * 10.0;
        }
        
        // Method 2: Fractional position strength (works for ALL numbers including primes)
        // Calculate how this position relates to common musical subdivisions
        double positionRatio = static_cast<double>(position) / stepCount;
        
        // Check alignment with common musical fractions
        double fractionStrengths[] = {
            1.0/2.0,  // Half (strongest secondary accent)
            1.0/4.0, 3.0/4.0,  // Quarters
            1.0/3.0, 2.0/3.0,  // Thirds
            1.0/8.0, 3.0/8.0, 5.0/8.0, 7.0/8.0,  // Eighths
            1.0/6.0, 5.0/6.0,  // Sixths
        };
        
        double fractionValues[] = {
            5.0,      // Half gets strong accent
            3.0, 3.0, // Quarters
            2.5, 2.5, // Thirds  
            1.5, 1.5, 1.5, 1.5, // Eighths
            1.0, 1.0  // Sixths
        };
        
        // Find closest musical fraction and assign its strength
        double closestDistance = 1.0;
        double fractionStrength = 0.0;
        
        for (int i = 0; i < 11; ++i) {
            double distance = std::abs(positionRatio - fractionStrengths[i]);
            if (distance < closestDistance) {
                closestDistance = distance;
                fractionStrength = fractionValues[i];
            }
        }
        
        // Apply fraction strength if it's close enough (tolerance for discrete positions)
        double tolerance = 0.5 / stepCount; // Half a step tolerance
        if (closestDistance <= tolerance) {
            indispensability = std::max(indispensability, fractionStrength);
        }
        
        // Method 3: Position-based weighting for remaining positions
        // Creates hierarchy based on distance from strong positions
        if (indispensability < 0.5) {
            // Distance from center (creates symmetrical hierarchy)
            double centerDistance = std::abs(position - stepCount / 2.0) / (stepCount / 2.0);
            
            // Distance from edges (pickup and anacrusis effects)
            double edgeDistance = std::min(position, stepCount - position) / (stepCount / 2.0);
            
            // Combine for unique values that avoid sequential filling
            indispensability = (1.0 - centerDistance * 0.3) + (edgeDistance * 0.2);
            
            // Add small position-dependent variation to break ties
            indispensability += (position % 3) * 0.01 + (position % 5) * 0.005;
        }
        
        // Special position bonuses
        if (position == stepCount - 1) {
            // Pickup beat (last position) gets high indispensability
            indispensability = std::max(indispensability, 7.0);
        }
        
        // Ensure all positions have unique values and avoid sequential patterns
        return std::max(indispensability, 0.1 + (position * 0.001));
    }
    
    //==============================================================================
    // Internal Helper Functions
    //==============================================================================
    
    namespace Internal
    {
        std::vector<bool> diluteByBarlow(const std::vector<bool>& pattern, 
                                        int targetOnsets,
                                        const std::vector<std::pair<int, double>>& indispensabilityTable,
                                        bool wolrabMode)
        {
            std::vector<bool> result = pattern;
            int currentOnsets = countOnsets(pattern);
            
            // Create list of current onset positions with their indispensabilities
            std::vector<std::pair<int, double>> onsetPositions;
            for (size_t i = 0; i < pattern.size(); ++i)
            {
                if (pattern[i])
                {
                    double indispensability = indispensabilityTable[i].second;
                    if (wolrabMode) indispensability = -indispensability; // Invert for Wolrab
                    onsetPositions.push_back({static_cast<int>(i), indispensability});
                }
            }
            
            // Sort by indispensability (lowest first for removal)
            std::sort(onsetPositions.begin(), onsetPositions.end(),
                     [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                         return a.second < b.second;
                     });
            
            // Remove onsets with lowest indispensability
            int onsetsToRemove = currentOnsets - targetOnsets;
            for (int i = 0; i < onsetsToRemove && i < onsetPositions.size(); ++i)
            {
                result[onsetPositions[i].first] = false;
            }
            
            return result;
        }
        
        std::vector<bool> concentrateByBarlow(const std::vector<bool>& pattern,
                                             int targetOnsets,
                                             const std::vector<std::pair<int, double>>& indispensabilityTable,
                                             bool wolrabMode)
        {
            std::vector<bool> result = pattern;
            int currentOnsets = countOnsets(pattern);
            
            // Create list of empty positions with their indispensabilities
            std::vector<std::pair<int, double>> emptyPositions;
            for (size_t i = 0; i < pattern.size(); ++i)
            {
                if (!pattern[i])
                {
                    double indispensability = indispensabilityTable[i].second;
                    if (wolrabMode) indispensability = -indispensability; // Invert for Wolrab
                    emptyPositions.push_back({static_cast<int>(i), indispensability});
                }
            }
            
            // Sort by indispensability (highest first for addition)
            std::sort(emptyPositions.begin(), emptyPositions.end(),
                     [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                         return a.second > b.second;
                     });
            
            // Add onsets with highest indispensability
            int onsetsToAdd = targetOnsets - currentOnsets;
            for (int i = 0; i < onsetsToAdd && i < emptyPositions.size(); ++i)
            {
                result[emptyPositions[i].first] = true;
            }
            
            return result;
        }
    }

} // namespace PatternUtils