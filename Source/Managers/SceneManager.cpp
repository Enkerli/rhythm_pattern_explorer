/*
  ==============================================================================

    SceneManager.cpp
    Implementation of scene cycling functionality
    
    CRITICAL: This implementation exactly replicates the original logic from
    PluginProcessor.cpp to ensure identical behavior.

  ==============================================================================
*/

#include "SceneManager.h"

//==============================================================================
void SceneManager::initializeScenes(const juce::StringArray& scenes)
{
    // Reset all scene state first
    resetScenes();
    
    // Copy the scenes
    scenePatterns = scenes;
    currentSceneIndex = 0;
    
    // Initialize progressive state for each scene - exact replica of original logic
    for (const auto& scene : scenes)
    {
        juce::String scenePattern = scene.trim();
        
        // Detection lives in the two helpers below, so the rules cannot drift
        // apart from each other. They used to be duplicated inline here, and
        // the copies disagreed: the '*' one never checked that what followed
        // was a number.
        const bool hasProgressiveOffset = sceneHasProgressiveOffset(scenePattern);
        const bool hasProgressiveLengthening = sceneHasProgressiveLengthening(scenePattern);
        
        if (hasProgressiveOffset) 
        {
            // Parse offset syntax: pattern%N or pattern+N - exact replica
            int symbolIndex = -1;
            if (scenePattern.contains("%")) {
                symbolIndex = scenePattern.lastIndexOf("%");
            } else if (scenePattern.contains("+")) {
                symbolIndex = scenePattern.lastIndexOf("+");
            }
            juce::String basePattern = scenePattern.substring(0, symbolIndex).trim();
            juce::String offsetStr = scenePattern.substring(symbolIndex + 1).trim();
            int step = offsetStr.getIntValue();
            
            sceneBasePatterns.push_back(basePattern);
            sceneProgressiveSteps.push_back(step);
            sceneProgressiveOffsets.push_back(step); // Start with first offset
            sceneProgressiveLengthening.push_back(0);
            sceneBaseLengthPatterns.push_back(std::vector<bool>());
        } 
        else if (hasProgressiveLengthening) 
        {
            // Parse lengthening syntax: pattern*N - exact replica
            int starIndex = scenePattern.lastIndexOf("*");
            juce::String basePattern = scenePattern.substring(0, starIndex).trim();
            juce::String lengthStr = scenePattern.substring(starIndex + 1).trim();
            int lengthStep = lengthStr.getIntValue();
            
            sceneBasePatterns.push_back(basePattern);
            sceneProgressiveSteps.push_back(lengthStep);
            sceneProgressiveOffsets.push_back(0);
            sceneProgressiveLengthening.push_back(lengthStep); // Start with first lengthening
            sceneBaseLengthPatterns.push_back(std::vector<bool>()); // Will be filled when pattern is generated
        } 
        else 
        {
            // Simple pattern without progressive syntax - exact replica
            sceneBasePatterns.push_back(scenePattern);
            sceneProgressiveSteps.push_back(0);
            sceneProgressiveOffsets.push_back(0);
            sceneProgressiveLengthening.push_back(0);
            sceneBaseLengthPatterns.push_back(std::vector<bool>());
        }
    }
}

void SceneManager::advanceScene()
{
    if (!scenePatterns.isEmpty() && currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size()))
    {
        // First, advance the progressive state for the current scene if it has progressive syntax
        // This is EXACTLY the original advanceScene() logic
        if (sceneProgressiveSteps[currentSceneIndex] != 0) 
        {
            if (sceneProgressiveOffsets[currentSceneIndex] != 0) 
            {
                // Progressive offset scene
                sceneProgressiveOffsets[currentSceneIndex] += sceneProgressiveSteps[currentSceneIndex];
            } 
            else if (sceneProgressiveLengthening[currentSceneIndex] != 0) 
            {
                // Progressive lengthening scene
                sceneProgressiveLengthening[currentSceneIndex] += sceneProgressiveSteps[currentSceneIndex];
            }
        }
        
        // Then advance to next scene in the sequence, cycling back to 0 when reaching the end
        // This is EXACTLY the original logic
        currentSceneIndex = (currentSceneIndex + 1) % scenePatterns.size();
    }
}

juce::String SceneManager::getCurrentScenePattern() const
{
    if (currentSceneIndex >= 0 && currentSceneIndex < scenePatterns.size())
    {
        return scenePatterns[currentSceneIndex];
    }
    return juce::String();
}

void SceneManager::resetScenes()
{
    currentSceneIndex = 0;
    scenePatterns.clear();
    sceneProgressiveOffsets.clear();
    sceneProgressiveSteps.clear();
    sceneBasePatterns.clear();
    sceneProgressiveLengthening.clear();
    sceneBaseLengthPatterns.clear();
}

//==============================================================================
// Persistence

namespace
{
    /** The format the processor used: values joined by commas, empty when none. */
    template <typename T, typename Fn>
    juce::String joinCsv(const T& items, Fn toString)
    {
        juce::String out;
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (i > 0) out += ",";
            out += toString(items[i]);
        }
        return out;
    }

    std::vector<int> splitInts(const juce::String& csv)
    {
        std::vector<int> out;
        if (csv.isEmpty()) return out;
        for (const auto& token : juce::StringArray::fromTokens(csv, ",", ""))
            out.push_back(token.getIntValue());
        return out;
    }
}

void SceneManager::saveStateTo(juce::ValueTree& state) const
{
    state.setProperty("currentSceneIndex", currentSceneIndex, nullptr);
    state.setProperty("sceneCount", static_cast<int>(scenePatterns.size()), nullptr);

    juce::String patterns;
    for (int i = 0; i < scenePatterns.size(); ++i)
    {
        if (i > 0) patterns += ",";
        patterns += scenePatterns[i];
    }
    state.setProperty("scenePatterns", patterns, nullptr);

    const auto intToString = [] (int v) { return juce::String(v); };
    state.setProperty("sceneProgressiveOffsets",    joinCsv(sceneProgressiveOffsets, intToString), nullptr);
    state.setProperty("sceneProgressiveSteps",      joinCsv(sceneProgressiveSteps, intToString), nullptr);
    state.setProperty("sceneBasePatterns",          joinCsv(sceneBasePatterns, [] (const juce::String& s) { return s; }), nullptr);
    state.setProperty("sceneProgressiveLengthening", joinCsv(sceneProgressiveLengthening, intToString), nullptr);
}

void SceneManager::restoreStateFrom(const juce::ValueTree& state)
{
    resetScenes();

    currentSceneIndex = state.getProperty("currentSceneIndex", 0);

    const juce::String patterns = state.getProperty("scenePatterns", juce::String());
    if (patterns.isNotEmpty())
        scenePatterns = juce::StringArray::fromTokens(patterns, ",", "");

    sceneProgressiveOffsets     = splitInts(state.getProperty("sceneProgressiveOffsets", juce::String()));
    sceneProgressiveSteps       = splitInts(state.getProperty("sceneProgressiveSteps", juce::String()));
    sceneProgressiveLengthening = splitInts(state.getProperty("sceneProgressiveLengthening", juce::String()));

    const juce::String basePatterns = state.getProperty("sceneBasePatterns", juce::String());
    if (basePatterns.isNotEmpty())
        for (const auto& p : juce::StringArray::fromTokens(basePatterns, ",", ""))
            sceneBasePatterns.push_back(p);

    // Lengthening base patterns are regenerated on first use, not persisted;
    // they only need to be the right size to be indexed safely.
    const int savedSceneCount = state.getProperty("sceneCount", 0);
    sceneBaseLengthPatterns.assign(static_cast<size_t>(juce::jmax(0, savedSceneCount)), std::vector<bool>());

    // A truncated or hand-edited session must not leave us indexing past the
    // end of the parallel arrays later.
    if (currentSceneIndex < 0 || currentSceneIndex >= scenePatterns.size())
        currentSceneIndex = 0;
}

//==============================================================================
// Per-Scene Progressive State Management

bool SceneManager::currentSceneHasProgressiveFeatures() const
{
    if (currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(sceneProgressiveSteps.size()))
        return false;
        
    return sceneProgressiveSteps[currentSceneIndex] != 0;
}

int SceneManager::getCurrentSceneProgressiveOffset() const
{
    if (currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(sceneProgressiveOffsets.size()))
        return 0;
        
    return sceneProgressiveOffsets[currentSceneIndex];
}

int SceneManager::getCurrentSceneProgressiveLengthening() const
{
    if (currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(sceneProgressiveLengthening.size()))
        return 0;
        
    return sceneProgressiveLengthening[currentSceneIndex];
}

juce::String SceneManager::getCurrentSceneBasePattern() const
{
    if (currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(sceneBasePatterns.size()))
        return juce::String();
        
    return sceneBasePatterns[currentSceneIndex];
}

std::vector<bool> SceneManager::getCurrentSceneBaseLengthPattern() const
{
    if (currentSceneIndex < 0 || currentSceneIndex >= static_cast<int>(sceneBaseLengthPatterns.size()))
        return std::vector<bool>();
        
    return sceneBaseLengthPatterns[currentSceneIndex];
}

void SceneManager::setCurrentSceneBaseLengthPattern(const std::vector<bool>& pattern)
{
    if (currentSceneIndex >= 0 && currentSceneIndex < static_cast<int>(sceneBaseLengthPatterns.size()))
    {
        sceneBaseLengthPatterns[currentSceneIndex] = pattern;
    }
}

//==============================================================================
// Helper Methods

bool SceneManager::sceneHasProgressiveOffset(const juce::String& scenePattern) const
{
    if ((scenePattern.contains("%") && scenePattern.lastIndexOf("%") > 0) || (scenePattern.contains("+") && scenePattern.lastIndexOf("+") > 0)) 
    {
        if (scenePattern.contains("%") && scenePattern.lastIndexOf("%") > 0) {
            int lastPercentIndex = scenePattern.lastIndexOf("%");
            juce::String afterPercent = scenePattern.substring(lastPercentIndex + 1).trim();
            return afterPercent.containsOnly("0123456789-") && afterPercent.isNotEmpty();
        } else if (scenePattern.contains("+") && scenePattern.lastIndexOf("+") > 0) {
            int lastPlusIndex = scenePattern.lastIndexOf("+");
            juce::String afterPlus = scenePattern.substring(lastPlusIndex + 1).trim();
            return afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty();
        }
    }
    return false;
}

bool SceneManager::sceneHasProgressiveLengthening(const juce::String& scenePattern) const
{
    // The tail after '*' must be a bare number, exactly as sceneHasProgressiveOffset
    // already requires of '%N'. Without this getIntValue() read "3/E(3,7)" as a
    // plain 3 and silently dropped the poly lane behind it.
    if (!scenePattern.contains("*") || scenePattern.lastIndexOf("*") <= 0) return false;
    const auto afterStar = scenePattern.substring(scenePattern.lastIndexOf("*") + 1).trim();
    return afterStar.containsOnly("0123456789") && afterStar.isNotEmpty();
}

juce::String SceneManager::getDebugInfo() const
{
    juce::String info;
    info << "SceneManager Debug Info:\\n";
    info << "  Scene Count: " << getSceneCount() << "\\n";
    info << "  Current Scene Index: " << currentSceneIndex << "\\n";
    
    for (int i = 0; i < scenePatterns.size(); ++i)
    {
        info << "  Scene " << i << ": " << scenePatterns[i] << "\\n";
        if (i < static_cast<int>(sceneBasePatterns.size()))
        {
            info << "    Base: " << sceneBasePatterns[i] << "\\n";
            info << "    Progressive Step: " << sceneProgressiveSteps[i] << "\\n";
            info << "    Current Offset: " << sceneProgressiveOffsets[i] << "\\n";
            info << "    Current Lengthening: " << sceneProgressiveLengthening[i] << "\\n";
        }
    }
    
    return info;
}