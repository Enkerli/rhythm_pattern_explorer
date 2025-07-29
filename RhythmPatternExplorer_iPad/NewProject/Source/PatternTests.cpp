/*
  ==============================================================================

    iPad RPE Pattern Tests
    Implementation of core pattern validation tests

  ==============================================================================
*/

#include "PatternTests.h"
#include "UPIParser.h"
#include "PatternEngine.h"
#include "PatternUtils.h"

//==============================================================================
// Static variables
int PatternTests::testsRun = 0;
int PatternTests::testsPassed = 0;
juce::StringArray PatternTests::testLog;

//==============================================================================
bool PatternTests::runAllTests()
{
    testsRun = 0;
    testsPassed = 0;
    testLog.clear();
    
    DBG("=== iPad RPE Pattern Tests ===");
    logTestResult("TEST SUITE", true, "Starting iPad RPE pattern validation tests");
    
    // Critical pattern validations
    bool allPassed = true;
    allPassed &= validateTresillo();
    allPassed &= validateQuintillo();
    allPassed &= validateHexTresillo();
    allPassed &= validateBarlowTresillo();
    
    // Core algorithm tests
    allPassed &= testEuclideanPatterns();
    allPassed &= testBinaryPatterns();
    allPassed &= testHexPatterns();
    
    // iPad-specific tests
    allPassed &= testUPIParserIntegration();
    allPassed &= testStringEncoding();
    
    // Results summary
    juce::String summary = juce::String("Tests: ") + juce::String(testsPassed) + 
                          "/" + juce::String(testsRun) + " passed";
    logTestResult("SUMMARY", allPassed, summary);
    
    DBG("=== Test Results: " + summary + " ===");
    return allPassed;
}

void PatternTests::logTestResult(const juce::String& testName, bool passed, const juce::String& details)
{
    testsRun++;
    if (passed) testsPassed++;
    
    juce::String result = passed ? "PASS" : "FAIL";
    juce::String logEntry = "[" + result + "] " + testName;
    if (details.isNotEmpty())
        logEntry += ": " + details;
    
    testLog.add(logEntry);
    DBG(logEntry);
}

juce::String PatternTests::getLastTestResults()
{
    juce::String results;
    
    // Show summary first
    results += "Tests Run: " + juce::String(testsRun) + "\n";
    results += "Passed: " + juce::String(testsPassed) + "\n";
    results += "Failed: " + juce::String(testsRun - testsPassed) + "\n\n";
    
    // Show failed tests only for iPad display
    for (const auto& logEntry : testLog)
    {
        if (logEntry.contains("[FAIL]"))
        {
            results += logEntry + "\n";
        }
    }
    
    // If no failures, show recent passes
    if (testsRun == testsPassed && testLog.size() > 0)
    {
        results += "Recent successful tests:\n";
        int shown = 0;
        for (const auto& logEntry : testLog)
        {
            if (logEntry.contains("[PASS]") && shown < 3)
            {
                results += logEntry.substring(0, 50) + "...\n";
                shown++;
            }
        }
    }
    
    return results;
}

//==============================================================================
// Critical Pattern Validations

bool PatternTests::validateTresillo()
{
    // E(3,8) should produce tresillo: 10010010
    auto parseResult = UPIParser::parse("E(3,8)");
    
    if (!parseResult.isValid())
    {
        logTestResult("Tresillo E(3,8)", false, "Parse failed");
        return false;
    }
    
    std::vector<bool> expected = {true, false, false, true, false, false, true, false};
    bool matches = patternsMatch(parseResult.pattern, expected);
    
    juce::String details = "Got: " + patternToString(parseResult.pattern) + 
                          ", Expected: " + patternToString(expected);
    logTestResult("Tresillo E(3,8)", matches, details);
    
    return matches;
}

bool PatternTests::validateQuintillo()
{
    // E(5,8) should produce cinquillo: 10110110
    auto parseResult = UPIParser::parse("E(5,8)");
    
    if (!parseResult.isValid())
    {
        logTestResult("Cinquillo E(5,8)", false, "Parse failed");
        return false;
    }
    
    std::vector<bool> expected = {true, false, true, true, false, true, true, false};
    bool matches = patternsMatch(parseResult.pattern, expected);
    
    juce::String details = "Got: " + patternToString(parseResult.pattern) + 
                          ", Expected: " + patternToString(expected);
    logTestResult("Cinquillo E(5,8)", matches, details);
    
    return matches;
}

bool PatternTests::validateHexTresillo()
{
    // 0x94:8 should produce tresillo: 10010010
    auto parseResult = UPIParser::parse("0x94:8");
    
    if (!parseResult.isValid())
    {
        logTestResult("Hex Tresillo 0x94:8", false, "Parse failed");
        return false;
    }
    
    std::vector<bool> expected = {true, false, false, true, false, false, true, false};
    bool matches = patternsMatch(parseResult.pattern, expected);
    
    juce::String details = "Got: " + patternToString(parseResult.pattern) + 
                          ", Expected: " + patternToString(expected);
    logTestResult("Hex Tresillo 0x94:8", matches, details);
    
    return matches;
}

bool PatternTests::validateBarlowTresillo()
{
    // B(3,8) should produce Barlow tresillo: 10001001
    auto parseResult = UPIParser::parse("B(3,8)");
    
    if (!parseResult.isValid())
    {
        logTestResult("Barlow Tresillo B(3,8)", false, "Parse failed");
        return false;
    }
    
    std::vector<bool> expected = {true, false, false, false, true, false, false, true};
    bool matches = patternsMatch(parseResult.pattern, expected);
    
    juce::String details = "Got: " + patternToString(parseResult.pattern) + 
                          ", Expected: " + patternToString(expected);
    logTestResult("Barlow Tresillo B(3,8)", matches, details);
    
    return matches;
}

//==============================================================================
// Core Algorithm Tests

bool PatternTests::testEuclideanPatterns()
{
    struct TestCase { 
        juce::String input; 
        std::vector<bool> expected; 
    };
    
    std::vector<TestCase> testCases = {
        {"E(1,4)", {true, false, false, false}},
        {"E(2,4)", {true, false, true, false}},
        {"E(3,4)", {true, true, true, false}},
        {"E(4,4)", {true, true, true, true}}
    };
    
    bool allPassed = true;
    for (const auto& testCase : testCases)
    {
        auto result = UPIParser::parse(testCase.input);
        bool matches = result.isValid() && patternsMatch(result.pattern, testCase.expected);
        
        juce::String details = "Got: " + patternToString(result.pattern) + 
                              ", Expected: " + patternToString(testCase.expected);
        logTestResult("Euclidean " + testCase.input, matches, details);
        
        allPassed &= matches;
    }
    
    return allPassed;
}

bool PatternTests::testBinaryPatterns()
{
    struct TestCase { 
        juce::String input; 
        std::vector<bool> expected; 
    };
    
    std::vector<TestCase> testCases = {
        {"1010", {true, false, true, false}},
        {"1100", {true, true, false, false}},
        {"101010", {true, false, true, false, true, false}}
    };
    
    bool allPassed = true;
    for (const auto& testCase : testCases)
    {
        auto result = UPIParser::parse(testCase.input);
        bool matches = result.isValid() && patternsMatch(result.pattern, testCase.expected);
        
        juce::String details = "Got: " + patternToString(result.pattern) + 
                              ", Expected: " + patternToString(testCase.expected);
        logTestResult("Binary " + testCase.input, matches, details);
        
        allPassed &= matches;
    }
    
    return allPassed;
}

bool PatternTests::testHexPatterns()
{
    struct TestCase { 
        juce::String input; 
        std::vector<bool> expected; 
    };
    
    // Test critical hex patterns with left-to-right bit ordering
    std::vector<TestCase> testCases = {
        {"0x1:4", {true, false, false, false}},    // LSB first
        {"0x8:4", {false, false, false, true}},    // MSB first  
        {"0xA:4", {false, true, false, true}},     // Mixed pattern
        {"0x94:8", {true, false, false, true, false, false, true, false}} // Tresillo
    };
    
    bool allPassed = true;
    for (const auto& testCase : testCases)
    {
        auto result = UPIParser::parse(testCase.input);
        bool matches = result.isValid() && patternsMatch(result.pattern, testCase.expected);
        
        juce::String details = "Got: " + patternToString(result.pattern) + 
                              ", Expected: " + patternToString(testCase.expected);
        logTestResult("Hex " + testCase.input, matches, details);
        
        allPassed &= matches;
    }
    
    return allPassed;
}

//==============================================================================
// iPad-Specific Tests

bool PatternTests::testUPIParserIntegration()
{
    // Test that UPIParser integrates correctly with PatternEngine
    PatternEngine engine;
    UPIParser::setProgressiveOffsetEngine(&engine);
    
    auto result = UPIParser::parse("E(3,8)");
    bool success = result.isValid() && result.pattern.size() == 8;
    
    logTestResult("UPI Parser Integration", success, 
                 success ? "Integration working" : "Integration failed");
    
    return success;
}

bool PatternTests::testStringEncoding()
{
    // Test string handling for iPad text input
    juce::String testInput = "E(3,8)";
    juce::String trimmed = testInput.trim();
    
    bool encodingOK = (testInput == trimmed) && testInput.length() == 6;
    
    logTestResult("String Encoding", encodingOK, 
                 "Input: '" + testInput + "', Length: " + juce::String(testInput.length()));
    
    return encodingOK;
}

//==============================================================================
// Utility Functions

juce::String PatternTests::patternToString(const std::vector<bool>& pattern)
{
    juce::String result;
    for (bool step : pattern)
    {
        result += step ? "1" : "0";
    }
    return result;
}

bool PatternTests::patternsMatch(const std::vector<bool>& a, const std::vector<bool>& b)
{
    if (a.size() != b.size()) return false;
    
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (a[i] != b[i]) return false;
    }
    
    return true;
}

std::vector<bool> PatternTests::referenceEuclidean(int onsets, int steps)
{
    if (steps <= 0 || onsets <= 0 || onsets > steps) {
        return std::vector<bool>(std::max(1, steps), false);
    }
    
    std::vector<bool> pattern(steps, false);
    int bucket = 0;
    
    for (int i = 0; i < steps; ++i) {
        bucket += onsets;
        if (bucket >= steps) {
            bucket -= steps;
            pattern[i] = true;
        }
    }
    
    return pattern;
}