/*
  ==============================================================================

    PatternUtils.h
    Centralized Pattern Manipulation Utilities
    
    Contains all pattern transformation, generation, and utility functions
    that were previously scattered across PatternEngine and UPIParser.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <random>

namespace PatternUtils
{
    //==============================================================================
    // Core Pattern Transformations
    //==============================================================================
    
    /** Inverts all bits in a pattern (true->false, false->true) */
    std::vector<bool> invertPattern(const std::vector<bool>& pattern);
    
    /** Reverses pattern order (mirrors the pattern) */
    std::vector<bool> reversePattern(const std::vector<bool>& pattern);
    
    /** Creates complement pattern (currently same as invert) */
    std::vector<bool> complementPattern(const std::vector<bool>& pattern);
    
    /** Rotates pattern by specified number of steps */
    std::vector<bool> rotatePattern(const std::vector<bool>& pattern, int steps);
    
    /** Combines two patterns using logical OR (addition) or AND-NOT (subtraction) */
    std::vector<bool> combinePatterns(const std::vector<bool>& pattern1, 
                                     const std::vector<bool>& pattern2, 
                                     bool isAddition = true);
    
    //==============================================================================
    // Pattern Generation Core Functions
    //==============================================================================
    
    /** Generates Euclidean patterns using Bjorklund algorithm */
    std::vector<bool> bjorklundAlgorithm(int onsets, int steps);
    
    /** Expands pattern to target size by repetition */
    std::vector<bool> expandToLCM(const std::vector<bool>& pattern, int targetSize);
    
    /** Stretches pattern to longer duration */
    std::vector<bool> dilatePattern(const std::vector<bool>& pattern, int newSteps);
    
    /** Compresses pattern to shorter duration */
    std::vector<bool> concentratePattern(const std::vector<bool>& pattern, int newSteps);
    
    //==============================================================================
    // Advanced Pattern Generation
    //==============================================================================
    
    /** Generates patterns using Barlow indispensability theory */
    std::vector<bool> generateBarlowTransformation(const std::vector<bool>& originalPattern, 
                                                  int targetOnsets, 
                                                  bool wolrabMode = false);
    
    /** Generates Euclidean or anti-Euclidean patterns */
    std::vector<bool> generateEuclideanTransformation(const std::vector<bool>& originalPattern,
                                                     int targetOnsets,
                                                     bool antiMode = false);
    
    /** Calculates metric strength using Barlow theory */
    double calculateBarlowIndispensability(int position, int stepCount);
    
    //==============================================================================
    // Pattern Utility Functions  
    //==============================================================================
    
    /** Counts true values in pattern */
    int countOnsets(const std::vector<bool>& pattern);
    
    /** Validates pattern is not empty */
    bool isValidPattern(const std::vector<bool>& pattern);
    
    /** Converts pattern to binary string representation */
    juce::String patternToBinary(const std::vector<bool>& pattern);
    
    //==============================================================================
    // Mathematical Utilities
    //==============================================================================
    
    /** Calculates greatest common divisor */
    int gcd(int a, int b);
    
    /** Calculates least common multiple */
    int lcm(int a, int b);
    
    /** Generates bell curve distributed onset count for random patterns */
    int bellCurveOnsetCount(int steps, std::mt19937& randomEngine);
    
    //==============================================================================
    // Pattern Format Conversion
    //==============================================================================
    
    /** Converts pattern to binary string */
    juce::String getBinaryString(const std::vector<bool>& pattern);
    
    /** Converts pattern to hexadecimal string with left-to-right notation */
    juce::String getHexString(const std::vector<bool>& pattern);
    
    /** Converts pattern to octal string */
    juce::String getOctalString(const std::vector<bool>& pattern);
    
    /** Converts pattern to decimal string */
    juce::String getDecimalString(const std::vector<bool>& pattern);
    
    //==============================================================================
    // Accent Pattern Functions
    //==============================================================================
    
    /** Checks if input contains accent pattern notation */
    bool hasAccentPattern(const juce::String& input);
    
    /** Extracts accent pattern from curly brackets */
    juce::String extractAccentPattern(const juce::String& input);
    
    /** Removes accent pattern notation from input */
    juce::String removeAccentPattern(const juce::String& input);
    
    //==============================================================================
    // String Processing Utilities
    //==============================================================================
    
    /** Trims and lowercases input string */
    juce::String cleanInput(const juce::String& input);
    
    /** Splits string by delimiter */
    juce::StringArray tokenize(const juce::String& input, const juce::String& delimiter);

    //==============================================================================
    // Internal Helper Functions (Private Implementation Details)
    //==============================================================================
    
    namespace Internal
    {
        /** Removes onsets based on Barlow indispensability */
        std::vector<bool> diluteByBarlow(const std::vector<bool>& pattern, 
                                        int targetOnsets,
                                        const std::vector<std::pair<int, double>>& indispensabilityTable,
                                        bool wolrabMode = false);
        
        /** Adds onsets based on Barlow indispensability */
        std::vector<bool> concentrateByBarlow(const std::vector<bool>& pattern,
                                             int targetOnsets,
                                             const std::vector<std::pair<int, double>>& indispensabilityTable,
                                             bool wolrabMode = false);
    }
}