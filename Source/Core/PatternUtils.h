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
    
    /** Calculate greatest common divisor (helper for Barlow algorithm) */
    int gcd(int a, int b);
    
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
    // Seeded RNG — the JS reference's, bit for bit
    //
    // `@enkerli/upi` seeds progressive lengthening from the pattern so far, so a
    // trigger names a pattern rather than a length (2026-08-02). These mirror
    // packages/upi/src/rng.js and rhythm.js EXACTLY so the plugin grows a `*N`
    // lane the same way the webapp and the CLI do — same mulberry32, same FNV-1a
    // seed, same Box-Muller, same Fisher-Yates, and the same NUMBER of draws in
    // the same order, which is what actually decides whether two streams agree.
    //
    // Deliberately not std::mt19937 / std::normal_distribution / std::shuffle:
    // all three are implementation-defined, so they would not even match across
    // two C++ standard libraries, let alone match JavaScript.
    //==============================================================================

    /** mulberry32 — packages/upi/src/rng.js. */
    struct Mulberry32
    {
        /** The seed is MIXED, exactly as rng.js does it — golden-ratio and
            a second constant for the `pass` term (0 here). Taking the seed raw
            produces a valid-looking but completely different stream, which is
            how this first went wrong. */
        explicit Mulberry32 (uint32_t seed, uint32_t pass = 0)
            : s (static_cast<uint32_t> (seed * 0x9e3779b1u + pass * 0x85ebca6bu)) {}
        /** One draw in [0,1), matching the JS bit pattern. */
        double next()
        {
            s = static_cast<uint32_t> (s + 0x6d2b79f5u);
            uint32_t t = s;
            t = static_cast<uint32_t> ((t ^ (t >> 15)) * (t | 1u));
            t ^= static_cast<uint32_t> (t + (t ^ (t >> 7)) * (t | 61u));
            return static_cast<double> ((t ^ (t >> 14)) >> 0) / 4294967296.0;
        }
        uint32_t s;
    };

    /** FNV-1a over the step bits, salted — rng.js seedFromSteps. */
    uint32_t seedFromSteps (const std::vector<bool>& steps, int salt);

    /** rhythm.js bellCurveRandomSteps, with an explicit seed. */
    std::vector<bool> bellCurveRandomSteps (int numSteps, uint32_t seed);
    
    //==============================================================================
    // Pattern Format Conversion
    //==============================================================================
    
    /** Converts pattern to binary string */
    juce::String getBinaryString(const std::vector<bool>& pattern);
    
    /** Converts pattern to hexadecimal string (leftmost = LSB; step k = bit k) */
    juce::String getHexString(const std::vector<bool>& pattern);
    
    /** Converts pattern to octal string */
    juce::String getOctalString(const std::vector<bool>& pattern);
    
    /** Converts pattern to decimal string */
    juce::String getDecimalString(const std::vector<bool>& pattern);
    
    
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