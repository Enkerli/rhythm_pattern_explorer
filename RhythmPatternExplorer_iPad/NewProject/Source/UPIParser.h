/*
  ==============================================================================

    Universal Pattern Input (UPI) Parser
    Based on the web application's pattern-processing.js

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <string>
#include <regex>
#include <map>

//==============================================================================
/**
 * Universal Pattern Input Parser
 * 
 * Supports the same pattern formats as the web application:
 * - Euclidean: E(3,8), E(5,16,2)
 * - Polygons: P(3,1), P(5,0,16)
 * - Binary: b10010010, 101010, 0x92:8
 * - Arrays: [0,3,6]:8
 * - Random: R(3,8), R(r,8)
 * - Transformations: ~pattern, rev pattern
 * - Combinations: P(3,1)+P(5,0)
 * - And many more...
 */
class UPIParser
{
public:
    struct ParseResult
    {
        enum Type { Single, Combination, Stringed, Error };
        
        Type type = Error;
        std::vector<bool> pattern;
        juce::String errorMessage;
        juce::String patternName;
        int stepCount = 0;
        
        // Progressive offset support
        bool hasProgressiveOffset = false;
        int initialOffset = 0;
        int progressiveOffset = 0;
        
        
        // Progressive transformation tracking
        juce::String progressivePatternKey;
        
        // Accent pattern support
        bool hasAccentPattern = false;
        std::vector<bool> accentPattern;
        juce::String accentPatternName;
        
        // Quantization support (Lascabettes-style)
        bool hasQuantization = false;
        int originalStepCount = 0;
        int quantizedStepCount = 0;
        bool quantizationClockwise = true;
        int originalOnsetCount = 0;
        int quantizedOnsetCount = 0;
        
        bool isValid() const { return type != Error; }
    };
    
    //==============================================================================
    // Main parsing functions
    static ParseResult parse(const juce::String& input);
    static ParseResult parsePattern(const juce::String& input);
    
    // Core pattern types
    static std::vector<bool> parseEuclidean(int onsets, int steps, int offset = 0);
    static std::vector<bool> parsePolygon(int sides, int offset, int totalSteps = 0);
    static std::vector<bool> parseBinary(const juce::String& binaryStr, int stepCount = 0);
    static std::vector<bool> parseArray(const juce::String& arrayStr, int stepCount = 0);
    static std::vector<bool> parseRandom(int onsets, int steps);
    static std::vector<bool> parseDecimal(int decimal, int stepCount);
    static std::vector<bool> parseMorse(const juce::String& morseStr);
    
    // Accent pattern parsing
    static std::vector<bool> parseAccentPattern(const juce::String& accentStr);
    
    // Pattern transformations and utilities are now centralized in PatternUtils namespace

private:
    // Pattern recognition - table-driven approach
    enum class PatternType {
        Euclidean, Polygon, Binary, Array, Random, Barlow, Wolrab, Dilcue, 
        Hex, Decimal, Octal, Morse
    };
    
    struct PatternRecognitionRule {
        juce::String startPrefix;
        juce::String endSuffix;
        juce::String alternateStart = "";
        bool (*customValidator)(const juce::String&) = nullptr;
    };
    
    static std::map<PatternType, PatternRecognitionRule> getPatternRules();
    static PatternType identifyPatternType(const juce::String& input);
    
    // Helper functions
    static juce::String cleanInput(const juce::String& input);
    static std::vector<juce::String> tokenize(const juce::String& input, const juce::String& delimiter);
    static ParseResult createError(const juce::String& message);
    static juce::String extractParameters(const juce::String& input, 
                                         const juce::String& prefix, 
                                         const juce::String& suffix = ")");
    
    // Progressive offset engine support
    static bool hasProgressiveOffsetEngine;
    static PatternEngine* progressiveOffsetEngine;
    
public:
    // Engine setup for progressive offset support
    static void setProgressiveOffsetEngine(PatternEngine* engine);
};