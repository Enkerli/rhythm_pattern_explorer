/*
  ==============================================================================

    AccentManager.cpp
    Implementation of accent pattern functionality
    
    CRITICAL: This implementation exactly replicates the original accent logic from
    PluginProcessor.cpp to ensure identical behavior.

  ==============================================================================
*/

#include "AccentManager.h"
#include "UPIParser.h"

//==============================================================================
bool AccentManager::parseAccentPattern(const juce::String& upiPattern)
{
    juce::String accentPart, mainPart;
    
    // Extract accent notation from UPI pattern
    if (!extractAccentNotation(upiPattern, accentPart, mainPart))
    {
        // No accent pattern found - clear existing accent
        clearAccentPattern();
        return false;
    }
    
    // Parse the extracted accent pattern
    if (parseAccentPatternString(accentPart))
    {
        hasActiveAccentPattern = true;
        currentAccentPatternName = accentPart;
        markPatternChanged();
        return true;
    }
    
    // Failed to parse accent pattern
    clearAccentPattern();
    return false;
}

void AccentManager::clearAccentPattern()
{
    hasActiveAccentPattern = false;
    currentAccentPattern.clear();
    currentAccentPatternName.clear();
    resetAccentPositions();
    markPatternChanged();
}

//==============================================================================
void AccentManager::advanceGlobalAccentPosition()
{
    if (hasActiveAccentPattern && !currentAccentPattern.empty())
    {
        globalAccentPosition = (globalAccentPosition + 1) % currentAccentPattern.size();
    }
}

void AccentManager::updateUIAccentOffsetAtCycleBoundary(int currentStep, int patternSize, int onsetsPerCycle)
{
    // Update UI accent offset only at cycle boundaries (step 0)
    if (currentStep == 0 && hasActiveAccentPattern && !currentAccentPattern.empty() && onsetsPerCycle > 0)
    {
        // Calculate how many complete pattern cycles we've been through
        // Use globalAccentPosition to determine UI offset for this cycle
        uiAccentOffset = globalAccentPosition % currentAccentPattern.size();
        markPatternChanged();
    }
}

void AccentManager::resetAccentPositions()
{
    globalAccentPosition = 0;
    uiAccentOffset = 0;
    markPatternChanged();
}

//==============================================================================
bool AccentManager::shouldOnsetBeAccented(int onsetIndex) const
{
    if (!hasActiveAccentPattern || currentAccentPattern.empty())
        return false;
    
    // Calculate what the global accent position will be for this onset
    int accentStep = (globalAccentPosition + onsetIndex) % currentAccentPattern.size();
    return currentAccentPattern[accentStep];
}

std::vector<bool> AccentManager::getCurrentAccentMap(const std::vector<bool>& rhythmPattern) const
{
    std::vector<bool> accentMap(rhythmPattern.size(), false);
    
    if (!hasActiveAccentPattern || currentAccentPattern.empty())
        return accentMap;
    
    // Apply accents only to onsets, not all steps
    int accentLen = static_cast<int>(currentAccentPattern.size());
    int onsetCounter = 0;
    
    for (size_t i = 0; i < rhythmPattern.size(); ++i)
    {
        if (rhythmPattern[i]) // Only process steps that contain onsets
        {
            int accentStep = (uiAccentOffset + onsetCounter) % accentLen;
            accentMap[i] = currentAccentPattern[accentStep];
            onsetCounter++;
        }
        // Steps without onsets remain false (unaccented)
    }
    
    return accentMap;
}

int AccentManager::getCurrentCycleAccentStart(int currentStep, int patternSize, int onsetsPerCycle) const
{
    if (!hasActiveAccentPattern || currentAccentPattern.empty() || patternSize == 0 || onsetsPerCycle == 0)
        return 0;
    
    // Calculate completed cycles and accent offset for this cycle
    int completedCycles = currentStep / patternSize;
    int cycleAccentOffset = (completedCycles * onsetsPerCycle) % currentAccentPattern.size();
    
    return cycleAccentOffset;
}

//==============================================================================
juce::String AccentManager::getDebugInfo() const
{
    juce::String info;
    info << "AccentManager Debug Info:\n";
    info << "  Has Accent Pattern: " << (hasActiveAccentPattern ? "Yes" : "No") << "\n";
    info << "  Accent Pattern Name: " << currentAccentPatternName << "\n";
    info << "  Accent Pattern Size: " << currentAccentPattern.size() << "\n";
    info << "  Global Accent Position: " << globalAccentPosition << "\n";
    info << "  UI Accent Offset: " << uiAccentOffset << "\n";
    info << "  Pattern Changed: " << (patternChanged ? "Yes" : "No") << "\n";
    
    if (hasActiveAccentPattern && !currentAccentPattern.empty())
    {
        info << "  Accent Pattern: ";
        for (bool accent : currentAccentPattern)
        {
            info << (accent ? "1" : "0");
        }
        info << "\n";
    }
    
    return info;
}

//==============================================================================
// Helper Methods

bool AccentManager::extractAccentNotation(const juce::String& upiPattern, juce::String& accentPart, juce::String& mainPart)
{
    // Look for curly bracket notation: {accent}pattern or pattern{accent}
    int openBrace = upiPattern.indexOf("{");
    int closeBrace = upiPattern.indexOf("}");
    
    if (openBrace == -1 || closeBrace == -1 || closeBrace <= openBrace)
        return false; // No valid curly bracket notation found
    
    // Extract accent pattern and main pattern
    accentPart = upiPattern.substring(openBrace + 1, closeBrace).trim();
    
    if (openBrace == 0)
    {
        // Format: {accent}pattern
        mainPart = upiPattern.substring(closeBrace + 1).trim();
    }
    else
    {
        // Format: pattern{accent}
        mainPart = upiPattern.substring(0, openBrace).trim();
    }
    
    return !accentPart.isEmpty() && !mainPart.isEmpty();
}

bool AccentManager::parseAccentPatternString(const juce::String& accentPatternStr)
{
    currentAccentPattern.clear();
    
    // Check for algorithmic pattern notation (E, P, B, W, D)
    if (accentPatternStr.contains("(") && accentPatternStr.contains(")"))
    {
        // Parse algorithmic notation like E(2,5), P(3,0), etc.
        char patternType = accentPatternStr[0];
        
        int openParen = accentPatternStr.indexOf("(");
        int comma = accentPatternStr.indexOf(",");
        int closeParen = accentPatternStr.indexOf(")");
        
        if (openParen > 0 && comma > openParen && closeParen > comma)
        {
            int onsets = accentPatternStr.substring(openParen + 1, comma).trim().getIntValue();
            int steps = accentPatternStr.substring(comma + 1, closeParen).trim().getIntValue();
            
            if (onsets > 0 && steps > 0 && onsets <= steps)
            {
                currentAccentPattern = generateAlgorithmicAccentPattern(patternType, onsets, steps);
                return !currentAccentPattern.empty();
            }
        }
    }
    else
    {
        // Parse binary string notation like "100", "10010", etc.
        if (accentPatternStr.containsOnly("01"))
        {
            for (int i = 0; i < accentPatternStr.length(); ++i)
            {
                currentAccentPattern.push_back(accentPatternStr[i] == '1');
            }
            return !currentAccentPattern.empty();
        }
    }
    
    return false; // Failed to parse
}

std::vector<bool> AccentManager::generateAlgorithmicAccentPattern(char patternType, int onsets, int steps)
{
    std::vector<bool> pattern;
    
    // Use UPIParser to generate the pattern based on type
    juce::String algorithmicPattern;
    algorithmicPattern << patternType << "(" << onsets << "," << steps << ")";
    
    try 
    {
        // Create a temporary UPIParser result to generate the pattern
        UPIParser::ParseResult result = UPIParser::parsePattern(algorithmicPattern);
        
        // Convert result to boolean vector
        if (!result.pattern.empty())
        {
            pattern = result.pattern;
        }
    }
    catch (...)
    {
        // If parsing fails, return empty pattern
        pattern.clear();
    }
    
    return pattern;
}