/*
  ==============================================================================

    ProgressiveManager.cpp
    Implementation of progressive pattern logic for Rhythm Pattern Explorer

  ==============================================================================
*/

#include "ProgressiveManager.h"
#include "PatternEngine.h"
#include "UPIParser.h"
#include "PatternUtils.h"

//==============================================================================
ProgressiveManager::ProgressiveManager()
    : randomGenerator(std::random_device{}())
{
}

ProgressiveManager::~ProgressiveManager() = default;

//==============================================================================
bool ProgressiveManager::hasAnyProgressiveFeatures(const juce::String& upiPattern) const
{
    return hasProgressiveOffset(upiPattern) || 
           hasProgressiveLengthening(upiPattern) || 
           hasProgressiveTransformation(upiPattern);
}

bool ProgressiveManager::hasProgressiveOffset(const juce::String& upiPattern) const
{
    // Look for +N pattern (N can be negative)
    if (upiPattern.contains("+"))
    {
        int lastPlusIndex = upiPattern.lastIndexOf("+");
        if (lastPlusIndex > 0)
        {
            juce::String afterPlus = upiPattern.substring(lastPlusIndex + 1).trim();
            return afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty();
        }
    }
    return false;
}

bool ProgressiveManager::hasProgressiveLengthening(const juce::String& upiPattern) const
{
    // Look for *N pattern
    if (upiPattern.contains("*"))
    {
        int lastStarIndex = upiPattern.lastIndexOf("*");
        if (lastStarIndex > 0)
        {
            juce::String afterStar = upiPattern.substring(lastStarIndex + 1).trim();
            return afterStar.containsOnly("0123456789") && afterStar.isNotEmpty();
        }
    }
    return false;
}

bool ProgressiveManager::hasProgressiveTransformation(const juce::String& upiPattern) const
{
    // Look for >N pattern
    if (upiPattern.contains(">"))
    {
        int lastAngleIndex = upiPattern.lastIndexOf(">");
        if (lastAngleIndex > 0)
        {
            juce::String afterAngle = upiPattern.substring(lastAngleIndex + 1).trim();
            return afterAngle.containsOnly("0123456789") && afterAngle.isNotEmpty();
        }
    }
    return false;
}

//==============================================================================
void ProgressiveManager::initializeProgressiveState(const juce::String& stateKey, 
                                                   const juce::String& basePattern,
                                                   const juce::String& progressivePattern)
{
    // Create or update state for this pattern
    ProgressiveState& state = progressiveStates[stateKey];
    state.basePattern = basePattern;
    state.fullPattern = progressivePattern;
    
    // Parse progressive notation
    parseProgressiveNotation(progressivePattern, state);
    
    // Update LRU tracking
    updateLRUAccess(stateKey);
    
    // Clean up old states if needed
    cleanupLRUStates();
}

juce::String ProgressiveManager::applyProgressive(const juce::String& stateKey, PatternEngine& patternEngine)
{
    auto it = progressiveStates.find(stateKey);
    if (it == progressiveStates.end())
        return {}; // No state found
    
    ProgressiveState& state = it->second;
    updateLRUAccess(stateKey);
    
    juce::String result = state.basePattern;
    
    // Apply progressive transformations in order
    if (state.hasOffset)
    {
        result = applyProgressiveOffset(state, patternEngine);
    }
    
    if (state.hasLengthening)
    {
        result = applyProgressiveLengthening(state);
    }
    
    if (state.hasTransformation)
    {
        result = applyProgressiveTransformation(state);
    }
    
    return result;
}

void ProgressiveManager::triggerProgressive(const juce::String& stateKey, PatternEngine& patternEngine)
{
    auto it = progressiveStates.find(stateKey);
    if (it == progressiveStates.end()) {
        return; // No state found
    }
    
    ProgressiveState& state = it->second;
    updateLRUAccess(stateKey);
    
    // Trigger each progressive type
    if (state.hasOffset)
    {
        state.currentOffset += state.offsetStep;
        state.triggerCount++;
        
        // PatternEngine integration will happen in applyProgressiveOffset()
    }
    
    if (state.hasLengthening)
    {
        // Generate new random steps and add to pattern
        std::vector<bool> newSteps = generateBellCurveRandomSteps(state.lengtheningStep);
        state.currentLengthenedPattern.insert(state.currentLengthenedPattern.end(), 
                                            newSteps.begin(), newSteps.end());
    }
    
    if (state.hasTransformation)
    {
        state.transformationStep++;
        // Progressive transformation logic would be implemented here
        // This is complex and would need to integrate with existing transformation code
    }
}

//==============================================================================
int ProgressiveManager::getProgressiveOffsetValue(const juce::String& stateKey) const
{
    auto it = progressiveStates.find(stateKey);
    return (it != progressiveStates.end() && it->second.hasOffset) ? it->second.currentOffset : 0;
}

bool ProgressiveManager::hasProgressiveState(const juce::String& stateKey) const
{
    return progressiveStates.find(stateKey) != progressiveStates.end();
}

juce::String ProgressiveManager::getBasePattern(const juce::String& stateKey) const
{
    auto it = progressiveStates.find(stateKey);
    return (it != progressiveStates.end()) ? it->second.basePattern : juce::String{};
}

void ProgressiveManager::resetProgressiveOffset(const juce::String& stateKey)
{
    auto it = progressiveStates.find(stateKey);
    if (it != progressiveStates.end() && it->second.hasOffset)
    {
        it->second.currentOffset = 0;
        it->second.triggerCount = 0;
        updateLRUAccess(stateKey);
    }
}

//==============================================================================
std::vector<bool> ProgressiveManager::getCurrentLengthenedPattern(const juce::String& stateKey) const
{
    auto it = progressiveStates.find(stateKey);
    if (it != progressiveStates.end() && it->second.hasLengthening)
    {
        return it->second.currentLengthenedPattern;
    }
    return {};
}

void ProgressiveManager::resetProgressiveLengthening(const juce::String& stateKey)
{
    auto it = progressiveStates.find(stateKey);
    if (it != progressiveStates.end() && it->second.hasLengthening)
    {
        it->second.currentLengthenedPattern = it->second.baseLengthPattern;
        updateLRUAccess(stateKey);
    }
}

//==============================================================================
int ProgressiveManager::getTransformationStepCount(const juce::String& stateKey) const
{
    auto it = progressiveStates.find(stateKey);
    return (it != progressiveStates.end() && it->second.hasTransformation) ? it->second.transformationStep : 0;
}

void ProgressiveManager::resetProgressiveTransformation(const juce::String& stateKey)
{
    auto it = progressiveStates.find(stateKey);
    if (it != progressiveStates.end() && it->second.hasTransformation)
    {
        it->second.transformationStep = 0;
        it->second.currentTransformedPattern.clear();
        updateLRUAccess(stateKey);
    }
}

//==============================================================================
void ProgressiveManager::resetAllProgressiveForPattern(const juce::String& stateKey)
{
    auto it = progressiveStates.find(stateKey);
    if (it != progressiveStates.end())
    {
        ProgressiveState& state = it->second;
        
        // Reset all progressive states
        state.currentOffset = 0;
        state.triggerCount = 0;
        state.currentLengthenedPattern = state.baseLengthPattern;
        state.transformationStep = 0;
        state.currentTransformedPattern.clear();
        
        updateLRUAccess(stateKey);
    }
}

void ProgressiveManager::clearAllProgressiveStates()
{
    progressiveStates.clear();
}

void ProgressiveManager::cleanupLRUStates()
{
    if (progressiveStates.size() <= MAX_PROGRESSIVE_STATES)
        return;
    
    // Find least recently used states and remove them
    std::vector<std::pair<juce::String, int>> accessCounts;
    for (const auto& pair : progressiveStates)
    {
        accessCounts.push_back({pair.first, pair.second.accessCount});
    }
    
    // Sort by access count (ascending = least used first)
    std::sort(accessCounts.begin(), accessCounts.end(), 
              [](const auto& a, const auto& b) { return a.second < b.second; });
    
    // Remove least used states until we're under the limit
    int toRemove = static_cast<int>(progressiveStates.size()) - MAX_PROGRESSIVE_STATES + 10; // Remove extra for buffer
    for (int i = 0; i < toRemove && i < accessCounts.size(); ++i)
    {
        progressiveStates.erase(accessCounts[i].first);
    }
}

//==============================================================================
int ProgressiveManager::getActiveStateCount() const
{
    return static_cast<int>(progressiveStates.size());
}

juce::String ProgressiveManager::getDebugInfo() const
{
    juce::String info;
    info << "ProgressiveManager Debug Info:\n";
    info << "  Active States: " << getActiveStateCount() << "/" << MAX_PROGRESSIVE_STATES << "\n";
    
    for (const auto& pair : progressiveStates)
    {
        const juce::String& key = pair.first;
        const ProgressiveState& state = pair.second;
        
        info << "  State \"" << key << "\":\n";
        info << "    Base Pattern: \"" << state.basePattern << "\"\n";
        info << "    Full Pattern: \"" << state.fullPattern << "\"\n";
        info << "    Has Offset: " << (state.hasOffset ? "Yes" : "No");
        if (state.hasOffset)
        {
            info << " (step: " << state.offsetStep << ", current: " << state.currentOffset 
                 << ", triggers: " << state.triggerCount << ")";
        }
        info << "\n";
        info << "    Has Lengthening: " << (state.hasLengthening ? "Yes" : "No");
        if (state.hasLengthening)
        {
            info << " (step: " << state.lengtheningStep << ", pattern length: " << state.currentLengthenedPattern.size() << ")";
        }
        info << "\n";
        info << "    Has Transformation: " << (state.hasTransformation ? "Yes" : "No");
        if (state.hasTransformation)
        {
            info << " (type: " << state.transformationType << ", target: " << state.targetOnsets 
                 << ", step: " << state.transformationStep << ")";
        }
        info << "\n";
        info << "    Access Count: " << state.accessCount << "\n";
    }
    
    return info;
}

//==============================================================================
void ProgressiveManager::parseProgressiveNotation(const juce::String& pattern, ProgressiveState& state)
{
    // Reset state
    state.hasOffset = false;
    state.hasLengthening = false;
    state.hasTransformation = false;
    
    // Parse progressive offset (+N)
    if (hasProgressiveOffset(pattern))
    {
        int lastPlusIndex = pattern.lastIndexOf("+");
        juce::String afterPlus = pattern.substring(lastPlusIndex + 1).trim();
        state.offsetStep = afterPlus.getIntValue();
        state.currentOffset = 0;
        state.triggerCount = 0;
        state.hasOffset = true;
    }
    
    // Parse progressive lengthening (*N)
    if (hasProgressiveLengthening(pattern))
    {
        int lastStarIndex = pattern.lastIndexOf("*");
        juce::String afterStar = pattern.substring(lastStarIndex + 1).trim();
        state.lengtheningStep = afterStar.getIntValue();
        
        // Initialize with base pattern for lengthening
        auto parseResult = UPIParser::parsePattern(state.basePattern);
        if (parseResult.type != UPIParser::ParseResult::Error && !parseResult.pattern.empty())
        {
            state.baseLengthPattern = parseResult.pattern;
            state.currentLengthenedPattern = parseResult.pattern;
        }
        
        state.hasLengthening = true;
    }
    
    // Parse progressive transformation (>N)
    if (hasProgressiveTransformation(pattern))
    {
        int lastAngleIndex = pattern.lastIndexOf(">");
        juce::String afterAngle = pattern.substring(lastAngleIndex + 1).trim();
        state.targetOnsets = afterAngle.getIntValue();
        state.transformationStep = 0;
        
        // Determine transformation type from base pattern
        juce::String baseUpper = state.basePattern.toUpperCase();
        if (baseUpper.startsWith("B("))
            state.transformationType = "B";
        else if (baseUpper.startsWith("W("))
            state.transformationType = "W";
        else if (baseUpper.startsWith("E("))
            state.transformationType = "E";
        else if (baseUpper.startsWith("D("))
            state.transformationType = "D";
        else
            state.transformationType = "E"; // Default
        
        state.hasTransformation = true;
    }
}

juce::String ProgressiveManager::applyProgressiveOffset(ProgressiveState& state, PatternEngine& patternEngine)
{
    // Restore original architecture: let PatternEngine handle the rotation
    // Just configure PatternEngine with the current offset state
    patternEngine.setProgressiveOffset(true, state.currentOffset, state.offsetStep);
    
    // Return the base pattern - PatternEngine will apply rotation internally
    return state.basePattern;
}

juce::String ProgressiveManager::applyProgressiveLengthening(ProgressiveState& state)
{
    // Apply lengthened pattern directly to PatternEngine (restore original architecture)
    if (!state.currentLengthenedPattern.empty())
    {
        // Note: This assumes the calling code will pass the PatternEngine reference
        // For now, we'll return a binary representation and let the caller apply it
        juce::String result;
        for (bool bit : state.currentLengthenedPattern)
        {
            result += bit ? "1" : "0";
        }
        return result;
    }
    return state.basePattern;
}

juce::String ProgressiveManager::applyProgressiveTransformation(ProgressiveState& state)
{
    // Progressive transformation is complex and would integrate with existing transformation logic
    // For now, return the base pattern
    return state.basePattern;
}

std::vector<bool> ProgressiveManager::generateBellCurveRandomSteps(int numSteps)
{
    if (numSteps <= 0) return {};
    
    std::vector<bool> steps(numSteps, false);
    
    // Use normal distribution centered around middle of array
    std::normal_distribution<double> distribution(numSteps / 2.0, numSteps / 6.0);
    
    // Generate random positions with bell curve weighting
    for (int i = 0; i < numSteps / 3; ++i) // Add about 1/3 density
    {
        int position = static_cast<int>(std::round(distribution(randomGenerator)));
        if (position >= 0 && position < numSteps)
        {
            steps[position] = true;
        }
    }
    
    return steps;
}

void ProgressiveManager::updateLRUAccess(const juce::String& stateKey)
{
    auto it = progressiveStates.find(stateKey);
    if (it != progressiveStates.end())
    {
        it->second.accessCount++;
        it->second.lastAccessed = juce::Time::getCurrentTime();
    }
}

juce::String ProgressiveManager::extractBasePattern(const juce::String& pattern) const
{
    juce::String base = pattern;
    
    // Remove progressive notation in reverse order (transformation, lengthening, offset)
    if (base.contains(">"))
    {
        int angleIndex = base.lastIndexOf(">");
        if (angleIndex > 0)
        {
            juce::String afterAngle = base.substring(angleIndex + 1).trim();
            if (afterAngle.containsOnly("0123456789") && afterAngle.isNotEmpty())
            {
                base = base.substring(0, angleIndex).trim();
            }
        }
    }
    
    if (base.contains("*"))
    {
        int starIndex = base.lastIndexOf("*");
        if (starIndex > 0)
        {
            juce::String afterStar = base.substring(starIndex + 1).trim();
            if (afterStar.containsOnly("0123456789") && afterStar.isNotEmpty())
            {
                base = base.substring(0, starIndex).trim();
            }
        }
    }
    
    if (base.contains("+"))
    {
        int plusIndex = base.lastIndexOf("+");
        if (plusIndex > 0)
        {
            juce::String afterPlus = base.substring(plusIndex + 1).trim();
            if (afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty())
            {
                base = base.substring(0, plusIndex).trim();
            }
        }
    }
    
    return base;
}

//==============================================================================
// State Serialization (Phase 4: Progressive State Persistence)

void ProgressiveManager::saveProgressiveStatesToValueTree(juce::ValueTree& stateTree) const
{
    // Create a child tree for progressive states
    auto progressiveTree = juce::ValueTree("ProgressiveStates");
    
    // Serialize each progressive state
    for (const auto& [patternKey, state] : progressiveStates)
    {
        auto stateNode = juce::ValueTree("ProgressiveState");
        stateNode.setProperty("patternKey", patternKey, nullptr);
        
        // Pattern info
        stateNode.setProperty("basePattern", state.basePattern, nullptr);
        stateNode.setProperty("fullPattern", state.fullPattern, nullptr);
        
        // Progressive Offset (+N)
        stateNode.setProperty("hasOffset", state.hasOffset, nullptr);
        stateNode.setProperty("offsetStep", state.offsetStep, nullptr);
        stateNode.setProperty("currentOffset", state.currentOffset, nullptr);
        stateNode.setProperty("triggerCount", state.triggerCount, nullptr);
        
        // Progressive Lengthening (*N)
        stateNode.setProperty("hasLengthening", state.hasLengthening, nullptr);
        stateNode.setProperty("lengtheningStep", state.lengtheningStep, nullptr);
        
        // Serialize baseLengthPattern as string
        juce::String baseLengthPatternString;
        for (int i = 0; i < state.baseLengthPattern.size(); ++i)
        {
            baseLengthPatternString += state.baseLengthPattern[i] ? "1" : "0";
        }
        stateNode.setProperty("baseLengthPattern", baseLengthPatternString, nullptr);
        
        // Serialize currentLengthenedPattern as string
        juce::String currentLengthenedPatternString;
        for (int i = 0; i < state.currentLengthenedPattern.size(); ++i)
        {
            currentLengthenedPatternString += state.currentLengthenedPattern[i] ? "1" : "0";
        }
        stateNode.setProperty("currentLengthenedPattern", currentLengthenedPatternString, nullptr);
        
        // Progressive Transformation (>N)
        stateNode.setProperty("hasTransformation", state.hasTransformation, nullptr);
        stateNode.setProperty("transformationType", state.transformationType, nullptr);
        stateNode.setProperty("targetOnsets", state.targetOnsets, nullptr);
        stateNode.setProperty("transformationStep", state.transformationStep, nullptr);
        
        // Serialize currentTransformedPattern as string
        juce::String currentTransformedPatternString;
        for (int i = 0; i < state.currentTransformedPattern.size(); ++i)
        {
            currentTransformedPatternString += state.currentTransformedPattern[i] ? "1" : "0";
        }
        stateNode.setProperty("currentTransformedPattern", currentTransformedPatternString, nullptr);
        
        // LRU tracking
        stateNode.setProperty("accessCount", state.accessCount, nullptr);
        stateNode.setProperty("lastAccessed", static_cast<int64_t>(state.lastAccessed.toMilliseconds()), nullptr);
        
        progressiveTree.appendChild(stateNode, nullptr);
    }
    
    stateTree.appendChild(progressiveTree, nullptr);
}

void ProgressiveManager::loadProgressiveStatesFromValueTree(const juce::ValueTree& stateTree)
{
    // Clear existing states
    progressiveStates.clear();
    
    // Find progressive states tree
    auto progressiveTree = stateTree.getChildWithName("ProgressiveStates");
    if (!progressiveTree.isValid())
        return;
    
    // Restore each progressive state
    for (int i = 0; i < progressiveTree.getNumChildren(); ++i)
    {
        auto stateNode = progressiveTree.getChild(i);
        if (!stateNode.hasType("ProgressiveState"))
            continue;
        
        juce::String patternKey = stateNode.getProperty("patternKey", juce::String());
        if (patternKey.isEmpty())
            continue;
        
        ProgressiveState state;
        
        // Pattern info
        state.basePattern = stateNode.getProperty("basePattern", juce::String());
        state.fullPattern = stateNode.getProperty("fullPattern", juce::String());
        
        // Progressive Offset (+N)
        state.hasOffset = stateNode.getProperty("hasOffset", false);
        state.offsetStep = stateNode.getProperty("offsetStep", 0);
        state.currentOffset = stateNode.getProperty("currentOffset", 0);
        state.triggerCount = stateNode.getProperty("triggerCount", 0);
        
        // Progressive Lengthening (*N)
        state.hasLengthening = stateNode.getProperty("hasLengthening", false);
        state.lengtheningStep = stateNode.getProperty("lengtheningStep", 0);
        
        // Restore baseLengthPattern from string
        juce::String baseLengthPatternString = stateNode.getProperty("baseLengthPattern", juce::String());
        state.baseLengthPattern.clear();
        for (int j = 0; j < baseLengthPatternString.length(); ++j)
        {
            state.baseLengthPattern.push_back(baseLengthPatternString[j] == '1');
        }
        
        // Restore currentLengthenedPattern from string
        juce::String currentLengthenedPatternString = stateNode.getProperty("currentLengthenedPattern", juce::String());
        state.currentLengthenedPattern.clear();
        for (int j = 0; j < currentLengthenedPatternString.length(); ++j)
        {
            state.currentLengthenedPattern.push_back(currentLengthenedPatternString[j] == '1');
        }
        
        // Progressive Transformation (>N)
        state.hasTransformation = stateNode.getProperty("hasTransformation", false);
        state.transformationType = stateNode.getProperty("transformationType", juce::String());
        state.targetOnsets = stateNode.getProperty("targetOnsets", 0);
        state.transformationStep = stateNode.getProperty("transformationStep", 0);
        
        // Restore currentTransformedPattern from string
        juce::String currentTransformedPatternString = stateNode.getProperty("currentTransformedPattern", juce::String());
        state.currentTransformedPattern.clear();
        for (int j = 0; j < currentTransformedPatternString.length(); ++j)
        {
            state.currentTransformedPattern.push_back(currentTransformedPatternString[j] == '1');
        }
        
        // LRU tracking
        state.accessCount = stateNode.getProperty("accessCount", 0);
        int64_t lastAccessedMs = stateNode.getProperty("lastAccessed", static_cast<int64_t>(0));
        state.lastAccessed = juce::Time(lastAccessedMs);
        
        // Add to map
        progressiveStates[patternKey] = std::move(state);
    }
}