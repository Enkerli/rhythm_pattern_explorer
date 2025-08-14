/*
  ==============================================================================

    Lascabettes-Style Quantization Engine
    Implements angular onset mapping for rhythmic deformation
    Port of webapp pattern-processing.js quantization algorithms

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <set>

//==============================================================================
/**
 * Lascabettes-style quantization engine for angular onset mapping
 * 
 * Implements the mathematical algorithm from Toussaint's research on 
 * angular onset redistribution. Provides both clockwise and counterclockwise
 * quantization for rhythmic deformation effects.
 * 
 * Usage examples:
 * - quantizePattern([1,0,0,1,0,0,1,0], 12, true)   // 8->12 steps clockwise
 * - quantizePattern([1,0,1,0,1,0,1,0], 6, false)   // 8->6 steps counterclockwise
 */
class QuantizationEngine
{
public:
    //==============================================================================
    // Core quantization result structure
    struct QuantizationResult
    {
        std::vector<bool> pattern;
        int originalStepCount = 0;
        int quantizedStepCount = 0;
        bool isClockwise = true;
        int originalOnsetCount = 0;
        int quantizedOnsetCount = 0;
        double densityRatio = 1.0;
        bool isValid = false;
        juce::String errorMessage;
        
        QuantizationResult() = default;
        QuantizationResult(const std::vector<bool>& p, int origSteps, int quantSteps, bool clockwise)
            : pattern(p), originalStepCount(origSteps), quantizedStepCount(quantSteps), 
              isClockwise(clockwise), isValid(true) 
        {
            originalOnsetCount = 0;
            quantizedOnsetCount = 0;
            
            // Count onsets in quantized pattern
            for (bool onset : pattern) {
                if (onset) quantizedOnsetCount++;
            }
            
            // Calculate density ratio
            if (origSteps > 0) {
                densityRatio = static_cast<double>(quantizedStepCount) / static_cast<double>(origSteps);
            }
        }
    };
    
    //==============================================================================
    // Main quantization functions
    
    /**
     * Quantize a rhythm pattern using angular mapping
     * 
     * @param originalPattern  The input pattern to quantize
     * @param newStepCount     Target number of steps (1-128)
     * @param clockwise        true = clockwise, false = counterclockwise
     * @return QuantizationResult with quantized pattern and metadata
     */
    static QuantizationResult quantizePattern(const std::vector<bool>& originalPattern, 
                                            int newStepCount, 
                                            bool clockwise = true);
    
    /**
     * Parse quantization notation from semicolon syntax
     * 
     * Examples:
     * - "pattern;12" -> {newSteps: 12, clockwise: true}
     * - "pattern;-8" -> {newSteps: 8, clockwise: false}
     * 
     * @param input  String containing semicolon notation
     * @return Parsed quantization parameters or invalid result
     */
    struct QuantizationParams
    {
        int newStepCount = 0;
        bool clockwise = true;
        bool isValid = false;
        juce::String patternPart;
        juce::String errorMessage;
    };
    
    static QuantizationParams parseQuantizationNotation(const juce::String& input);
    
    /**
     * Check if input contains quantization notation
     * 
     * @param input  String to check for semicolon syntax
     * @return true if semicolon notation detected
     */
    static bool hasQuantizationNotation(const juce::String& input);
    
    //==============================================================================
    // Utility functions
    
    /**
     * Validate step count for quantization
     * 
     * @param stepCount  Step count to validate
     * @return true if valid (1-128), false otherwise
     */
    static bool isValidStepCount(int stepCount);
    
    /**
     * Calculate quality metrics for quantization
     * 
     * @param original     Original pattern
     * @param quantized    Quantized pattern  
     * @return Quality analysis struct
     */
    struct QualityMetrics
    {
        double onsetPreservationRatio = 0.0;
        double densityChange = 0.0;
        int conflictCount = 0;
        bool hasConflicts = false;
    };
    
    static QualityMetrics calculateQualityMetrics(const std::vector<bool>& original,
                                                 const QuantizationResult& quantized);

private:
    //==============================================================================
    // Internal algorithm implementation
    
    /**
     * Extract onset positions from binary pattern
     * 
     * @param pattern  Binary pattern to analyze
     * @return Vector of onset position indices
     */
    static std::vector<int> extractOnsetPositions(const std::vector<bool>& pattern);
    
    /**
     * Convert step position to angular position
     * 
     * @param position     Step position (0-based)
     * @param stepCount    Total number of steps
     * @param clockwise    Direction of mapping
     * @return Angular position in radians (0 to 2*PI)
     */
    static double positionToAngle(int position, int stepCount, bool clockwise);
    
    /**
     * Convert angular position back to step position
     * 
     * @param angle        Angular position in radians
     * @param stepCount    Target number of steps
     * @return Step position (0-based, bounded to stepCount)
     */
    static int angleToPosition(double angle, int stepCount);
    
    /**
     * Create error result with message
     * 
     * @param message  Error description
     * @return Invalid QuantizationResult with error message
     */
    static QuantizationResult createError(const juce::String& message);
    
    //==============================================================================
    // Mathematical constants and utilities
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double TWO_PI = 2.0 * PI;
    static constexpr int MIN_STEP_COUNT = 1;
    static constexpr int MAX_STEP_COUNT = 128;
    
    /**
     * Normalize angle to [0, 2*PI) range
     * 
     * @param angle  Input angle in radians
     * @return Normalized angle in [0, 2*PI)
     */
    static double normalizeAngle(double angle);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantizationEngine)
};