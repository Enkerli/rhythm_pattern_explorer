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
        
        // Accent pattern support
        bool hasAccentPattern = false;
        std::vector<bool> accentPattern;
        juce::String accentPatternName;
        
        // Progressive transformation tracking
        juce::String progressivePatternKey;
        
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
    
    // Pattern transformations
    static std::vector<bool> invertPattern(const std::vector<bool>& pattern);
    static std::vector<bool> reversePattern(const std::vector<bool>& pattern);
    static std::vector<bool> complementPattern(const std::vector<bool>& pattern);
    static std::vector<bool> rotatePattern(const std::vector<bool>& pattern, int steps);
    
    // Pattern combinations
    static std::vector<bool> combinePatterns(const std::vector<bool>& pattern1, 
                                           const std::vector<bool>& pattern2, 
                                           bool isAddition = true);
    
    // Utility functions
    static std::vector<bool> bjorklundAlgorithm(int onsets, int steps);
    static int gcd(int a, int b);
    static int lcm(int a, int b);
    static std::vector<bool> expandToLCM(const std::vector<bool>& pattern, int targetSize);
    
    // Pattern validation and analysis
    static bool isValidPattern(const std::vector<bool>& pattern);
    static int countOnsets(const std::vector<bool>& pattern);
    static juce::String patternToBinary(const std::vector<bool>& pattern);

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
    
    static bool isPatternType(const juce::String& input, PatternType type);
    static const std::map<PatternType, PatternRecognitionRule>& getPatternRules();
    
    // Legacy pattern recognition helpers (for backward compatibility)
    static bool isEuclideanPattern(const juce::String& input);
    static bool isPolygonPattern(const juce::String& input);
    static bool isBinaryPattern(const juce::String& input);
    static bool isArrayPattern(const juce::String& input);
    static bool isRandomPattern(const juce::String& input);
    static bool isBarlowPattern(const juce::String& input);
    static bool isWolrabPattern(const juce::String& input);
    static bool isDilcuePattern(const juce::String& input);
    static bool isHexPattern(const juce::String& input);
    static bool isDecimalPattern(const juce::String& input);
    static bool isOctalPattern(const juce::String& input);
    static bool isMorsePattern(const juce::String& input);
    
    // Generic numeric pattern handler
    enum class NumericBase { Binary = 2, Octal = 8, Decimal = 10, Hexadecimal = 16 };
    struct NumericPatternInfo {
        juce::String prefix;
        NumericBase base;
        juce::String validChars;
    };
    static bool isNumericPattern(const juce::String& input, const NumericPatternInfo& info);
    static ParseResult parseNumericPattern(const juce::String& input, const NumericPatternInfo& info, int stepCount);
    
    // Polygon combination helper
    static ParseResult parsePolygonForCombination(const juce::String& polygonStr, int targetSteps);
    
    // Progressive transformation helpers
    static std::vector<bool> applyProgressiveTransformation(const std::vector<bool>& basePattern, char transformerType, int targetOnsets);
    static std::vector<bool> dilatePattern(const std::vector<bool>& pattern, int newSteps);
    static std::vector<bool> concentratePattern(const std::vector<bool>& pattern, int newSteps);
    static std::vector<bool> generateBarlowTransformation(const std::vector<bool>& originalPattern, int targetOnsets, bool wolrabMode);
    static std::vector<bool> generateEuclideanTransformation(const std::vector<bool>& originalPattern, int targetOnsets, bool antiMode);
    static std::vector<bool> diluteByBarlow(const std::vector<bool>& pattern, int targetOnsets, const std::vector<std::pair<int, double>>& indispensabilityTable, bool wolrabMode);
    static std::vector<bool> concentrateByBarlow(const std::vector<bool>& pattern, int targetOnsets, const std::vector<std::pair<int, double>>& indispensabilityTable, bool wolrabMode);
    static double calculateBarlowIndispensability(int position, int stepCount);
    static void resetProgressiveState(const juce::String& patternKey);
    
public:
    static void resetAllProgressiveStates();
    static int getProgressiveStepCount(const juce::String& patternKey);
    
    // Progressive offset engine support
    static void setProgressiveOffsetEngine(class PatternEngine* engine);
    static int getCurrentProgressiveOffset();
    static bool hasProgressiveOffsetEngine;
    static class PatternEngine* progressiveOffsetEngine;
    
    // String processing utilities
    static juce::String cleanInput(const juce::String& input);
    static juce::StringArray tokenize(const juce::String& input, const juce::String& delimiter);
    static bool hasTransformationPrefix(const juce::String& input);
    
    // Accent pattern utilities
    static bool hasAccentPattern(const juce::String& input);
    static juce::String extractAccentPattern(const juce::String& input);
    static juce::String removeAccentPattern(const juce::String& input);
    static ParseResult parseAccentPattern(const juce::String& accentStr);
    
    // Error handling
    static ParseResult createError(const juce::String& message);
    static ParseResult createSuccess(const std::vector<bool>& pattern, const juce::String& name = "");
};