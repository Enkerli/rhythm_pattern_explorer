/*
  ==============================================================================

    ProgressiveTransformState.cpp
    See ProgressiveTransformState.h. The map operations are lifted verbatim
    from the file-scope statics they replace (UPIParser.cpp's
    cleanupProgressiveStates / resetProgressiveState / resetAllProgressiveStates
    / getProgressiveStepCount) so that moving the state changes WHO OWNS IT and
    nothing else — the eviction order, the 1-based step count and the
    "unknown key means step 1" default all behave as before.

  ==============================================================================
*/

#include "ProgressiveTransformState.h"
#include <algorithm>

namespace
{
    // One entry per key, as "key:bits:step:access", joined by '\n'. Flat string
    // properties are the idiom already used for scenes and accent patterns in
    // this processor's state tree; a key can hold any pattern text, so the
    // field separator has to be one the key cannot contain, and ':' cannot
    // appear in a key (which is bits + a letter + digits).
    juce::String bitsOf (const std::vector<bool>& p)
    {
        juce::String s;
        for (bool b : p) s << (b ? '1' : '0');
        return s;
    }
}

//==============================================================================
void ProgressiveTransformState::evictIfNeeded()
{
    if (static_cast<int> (patterns.size()) <= maxStates) return;

    std::vector<std::pair<int, juce::String>> accessCounts;
    accessCounts.reserve (accessCount.size());
    for (const auto& pair : accessCount)
        accessCounts.push_back ({ pair.second, pair.first });

    std::sort (accessCounts.begin(), accessCounts.end());

    const int toRemove = static_cast<int> (patterns.size()) - (maxStates / 2);
    for (int i = 0; i < toRemove && i < static_cast<int> (accessCounts.size()); ++i)
    {
        const auto& keyToRemove = accessCounts[static_cast<size_t> (i)].second;
        patterns.erase (keyToRemove);
        accessCount.erase (keyToRemove);
        stepCount.erase (keyToRemove);
    }
}

void ProgressiveTransformState::forget (const juce::String& key)
{
    patterns.erase (key);
    accessCount.erase (key);
    stepCount.erase (key);
}

void ProgressiveTransformState::clear()
{
    patterns.clear();
    accessCount.clear();
    stepCount.clear();
}

int ProgressiveTransformState::stepCountFor (const juce::String& key) const
{
    const auto it = stepCount.find (key);
    return it != stepCount.end() ? it->second : 1; // unknown key: step 1, the base
}

//==============================================================================
void ProgressiveTransformState::saveTo (juce::ValueTree& state) const
{
    juce::StringArray rows;
    for (const auto& entry : patterns)
    {
        const auto step   = stepCountFor (entry.first);
        const auto access = accessCount.count (entry.first) ? accessCount.at (entry.first) : 0;
        rows.add (entry.first + ":" + bitsOf (entry.second)
                    + ":" + juce::String (step) + ":" + juce::String (access));
    }
    state.setProperty ("progressiveTransformState", rows.joinIntoString ("\n"), nullptr);
}

void ProgressiveTransformState::restoreFrom (const juce::ValueTree& state)
{
    clear();
    const juce::String saved = state.getProperty ("progressiveTransformState", juce::String());
    if (saved.isEmpty()) return;

    for (const auto& row : juce::StringArray::fromLines (saved))
    {
        if (row.trim().isEmpty()) continue;
        auto fields = juce::StringArray::fromTokens (row, ":", "");
        if (fields.size() != 4) continue; // a malformed row is skipped, not guessed at

        const juce::String key = fields[0];
        std::vector<bool> pattern;
        for (int i = 0; i < fields[1].length(); ++i)
            pattern.push_back (fields[1][i] == '1');

        patterns[key]    = pattern;
        stepCount[key]   = fields[2].getIntValue();
        accessCount[key] = fields[3].getIntValue();
    }
}
