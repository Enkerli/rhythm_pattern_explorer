/*
  ==============================================================================

    Rhythm Pattern Explorer - Pattern Engine
    Core rhythm pattern generation and analysis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <random>

//==============================================================================
/**
 * Core pattern generation engine
 * 
 * Implements the mathematical algorithms from the web app:
 * - Euclidean rhythms (Bjorklund algorithm)
 * - Regular polygon patterns
 * - Random patterns with bell curve distribution
 * - Binary pattern representation
 * - Perfect balance analysis (Milne's theory)
 */
class PatternEngine
{
public:
    PatternEngine();
    ~PatternEngine();
    
    //==============================================================================
    // Pattern Generation
    void generateEuclideanPattern(int onsets, int steps, int offset = 0);
    void generatePolygonPattern(int vertices, int steps, int offset = 0);
    void generateRandomPattern(int onsets, int steps);
    void generateBinaryPattern(int binaryValue, int steps);
    
    //==============================================================================
    // Pattern Access
    const std::vector<bool>& getCurrentPattern() const { return currentPattern; }
    void setPattern(const std::vector<bool>& pattern);
    int getStepCount() const { return static_cast<int>(currentPattern.size()); }
    int getOnsetCount() const;
    
    //==============================================================================
    // Pattern Analysis (simplified versions from web app)
    struct BalanceAnalysis
    {
        double magnitude = 0.0;
        double normalizedMagnitude = 0.0;
        bool isPerfectlyBalanced = false;
        juce::String balanceRating = "Unknown";
    };
    
    BalanceAnalysis analyzeBalance() const;
    double calculateCenterOfGravityAngle() const;
    
    //==============================================================================
    // Pattern Information
    juce::String getPatternDescription() const;
    juce::String getBinaryString() const;
    juce::String getHexString() const;
    juce::String getOctalString() const;
    juce::String getDecimalString() const;
    
    //==============================================================================
    // Progressive Offset Support
    void setProgressiveOffset(bool enabled, int initial = 0, int progressive = 0);
    void triggerProgressiveOffset();
    int getCurrentOffset() const { return currentOffset; }
    bool hasProgressiveOffsetEnabled() const { return hasProgressiveOffset; }
    int getProgressiveTriggerCount() const { return triggerCount; }
    int getProgressiveOffsetValue() const { return progressiveOffset; }

private:
    //==============================================================================
    std::vector<bool> currentPattern;
    std::mt19937 randomEngine;
    
    // Progressive offset state
    bool hasProgressiveOffset = false;
    int initialOffset = 0;
    int progressiveOffset = 0;
    int currentOffset = 0;
    int triggerCount = 0;
    
    //==============================================================================
    // Helper methods
    std::vector<bool> euclideanDistribution(int onsets, int steps);
    // Pattern utility functions moved to UPIParser for centralization
    int bellCurveOnsetCount(int steps);
    double calculateGCD(double a, double b);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternEngine)
};