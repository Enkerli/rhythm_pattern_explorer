/*
  ==============================================================================

    Universal Pattern Input (UPI) Parser
    Based on the web application's pattern-processing.js

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ProgressiveTransformState.h"
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
 * - Polygons: P(3,1), P(3,1,4)  (3rd arg = expansion factor: sides * factor steps)
 * - Binary: b10010010, 101010, 0x92:8
 * - Arrays: [0,3,6]:8
 * - Random: R(3,8), R(r,8)
 * - Morse: M:SOS, L:1,3 .-, M:SOS{l}, M:HELP{w}
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
        
        // Feel suffixes (music-suite docs/SERPE_RECOVERY.md). Stripped before
        // any other parsing — exactly as the JS does — so their contents can
        // never reach the '+'/'-' combination splitter.
        //   PD(20%) / PD(0.25, seed)   microtiming: WHERE attacks land
        //   LS(3) / LS(1.4..1.8, 70%)  note LENGTH (articulation): how long
        bool hasMicrotiming = false;
        double microtimingDepth = 0.0;
        int microtimingSeed = 1;

        bool hasLongShort = false;
        double longShortMin = 1.0;
        double longShortMax = 1.0;
        double longShortDepth = 0.0;

        bool isValid() const { return type != Error; }
    };
    
    //==============================================================================
    // Main parsing functions
    /**
     * Parse a UPI string. `progressive` is the caller's OWN `>N` bookkeeping —
     * the processor's for a mono pattern, the lane's own for a poly lane.
     *
     * NO DEFAULT ARGUMENT, on purpose (ProgressiveTransformState.h, INTENT L5):
     * a defaulted parameter would let a future call site fall back to shared
     * state without anyone noticing, which is the shape of the bug this
     * parameter exists to kill. Two call sites is a cheap price for the
     * compiler visiting both.
     */
    static ParseResult parse(const juce::String& input, ProgressiveTransformState& progressive);

    /**
     * One pattern term. Takes the same state, because the `>N` progressive
     * branch lives here rather than in parseAfterFeel — and because this
     * function recurses into itself for the named patterns (tresillo, hex, …).
     */
    static ParseResult parsePattern(const juce::String& input, ProgressiveTransformState& progressive);

    // Everything parse() does AFTER the feel suffixes have been stripped.
    // Split out so parse() has exactly one return statement and can carry the
    // feel flags onto whichever of this function's many returns wins — they
    // used to be recorded on a local that was then thrown away, which is why
    // PD()/LS() parsed cleanly and then did nothing in every DAW.
    static ParseResult parseAfterFeel(const juce::String& cleanedInput, ProgressiveTransformState& progressive);

    // Feel suffixes — strip and record; return the remaining pattern text.
    static juce::String extractMicrotiming(const juce::String& input, ParseResult& result);
    static juce::String extractLongShort(const juce::String& input, ParseResult& result);
    
    // Core pattern types
    static std::vector<bool> parseEuclidean(int onsets, int steps, int offset = 0);
    static std::vector<bool> parsePolygon(int sides, int offset, int totalSteps = 0);
    static std::vector<bool> parseBinary(const juce::String& binaryStr, int stepCount = 0);
    static std::vector<bool> parseArray(const juce::String& arrayStr, int stepCount = 0);
    static std::vector<bool> parseRandom(int onsets, int steps);
    static std::vector<bool> parseDecimal(uint64_t decimal, int stepCount);  // 64-bit: long patterns
    static std::vector<bool> parseMorse(const juce::String& morseStr);
    static std::vector<bool> parseMorseWithDurations(const juce::String& morseStr, int shortDuration, int longDuration);
    static std::pair<std::vector<bool>, std::vector<bool>> parseMorseWithAccents(const juce::String& morseStr, char accentMode, int shortDuration = 1, int longDuration = 2);
    
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
    
    // Progressive transformation helpers. The state is the caller's — see parse().
    static std::vector<bool> applyProgressiveTransformation(const std::vector<bool>& basePattern, char transformerType, int targetOnsets, ProgressiveTransformState& progressive);
    static std::vector<bool> diluteByBarlow(const std::vector<bool>& pattern, int targetOnsets, const std::vector<std::pair<int, double>>& indispensabilityTable, bool wolrabMode);
    static std::vector<bool> concentrateByBarlow(const std::vector<bool>& pattern, int targetOnsets, const std::vector<std::pair<int, double>>& indispensabilityTable, bool wolrabMode);
    static double calculateBarlowIndispensability(int position, int stepCount);

public:
    // resetProgressiveState / resetAllProgressiveStates / getProgressiveStepCount
    // used to live here and reach into the process-wide maps. They are now
    // ProgressiveTransformState::forget / clear / stepCountFor, called on the
    // state you own — there is no longer any "all" to reset, which is the point.

    // Progressive offset engine support
    static void setProgressiveOffsetEngine(class PatternEngine* engine);
    static int getCurrentProgressiveOffset();
    static bool hasProgressiveOffsetEngine;
    static class PatternEngine* progressiveOffsetEngine;
    
    // String processing utilities
    static juce::String cleanInput(const juce::String& input);
    static juce::StringArray tokenize(const juce::String& input, const juce::String& delimiter);
    static bool hasTransformationPrefix(const juce::String& input);
    
    
    // Error handling
    static ParseResult createError(const juce::String& message);
    static ParseResult createSuccess(const std::vector<bool>& pattern, const juce::String& name = "");
};