/*
  ==============================================================================

    iPad RPE Pattern Tests
    Core pattern validation tests for iPad AUv3 plugin
    
    Essential tests to ensure pattern generation accuracy matches desktop version.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cassert>

//==============================================================================
/**
 * Core Pattern Tests for iPad RPE
 * 
 * Tests critical pattern generation algorithms to ensure mathematical accuracy
 * and cross-platform consistency with desktop version.
 */
class PatternTests
{
public:
    //==============================================================================
    // Test suite execution
    static bool runAllTests();
    static void logTestResult(const juce::String& testName, bool passed, const juce::String& details = "");
    static juce::String getLastTestResults();
    
    //==============================================================================
    // Core algorithm tests
    static bool testEuclideanPatterns();
    static bool testBinaryPatterns(); 
    static bool testHexPatterns();
    static bool testOctalPatterns();
    static bool testBarlowPatterns();
    static bool testProgressiveTransformations();
    
    //==============================================================================
    // iPad-specific tests
    static bool testUPIParserIntegration();
    static bool testStringEncoding();
    static bool testPatternVisualization();
    
    //==============================================================================
    // Critical pattern validations
    static bool validateTresillo();           // E(3,8) = 10010010
    static bool validateQuintillo();          // E(5,8) = 10101010
    static bool validateHexTresillo();        // 0x94:8 = 10010010
    static bool validateBarlowTresillo();     // B(3,8) = 10001010
    
    //==============================================================================
    // Utility functions
    static juce::String patternToString(const std::vector<bool>& pattern);
    static bool patternsMatch(const std::vector<bool>& a, const std::vector<bool>& b);
    static std::vector<bool> referenceEuclidean(int onsets, int steps);
    
private:
    static int testsRun;
    static int testsPassed;
    static juce::StringArray testLog;
};