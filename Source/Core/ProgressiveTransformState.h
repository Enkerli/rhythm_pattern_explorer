/*
  ==============================================================================

    ProgressiveTransformState.h
    Where a `>N` progressive transformation has got to, for each pattern key.

    This is a THING SOMEBODY OWNS, and that is the entire point. It used to be
    three file-scope statics in UPIParser.cpp, keyed only by the pattern text:

        static std::map<juce::String, std::vector<bool>> progressivePatterns;
        static std::map<juce::String, int> progressiveAccessCount;
        static std::map<juce::String, int> progressiveStepCount;

    One map per PROCESS meant every Serpe instance in a DAW shared one counter
    per pattern string. Two tracks running E(1,8)>8 fought over it; a new
    project inherited whatever the last one left behind; and the project file
    could not reach the authoritative copy, so saving a session did not save
    what it played. Measured 2026-08-01 — two instances triggered alternately
    took alternate steps of one sequence, and two IDENTICAL poly lanes came
    apart on trigger 1 (music-suite docs/SERPE_DAW_FINDINGS_2026-08.md F1/F1a).

    Ownership now: the processor holds one for mono, and each PolyLaneRuntime
    holds its own — which is what makes two lanes over the same text independent
    rather than in a fight. UPIParser::parse takes one BY REFERENCE and has no
    default argument, deliberately: a defaulted parameter would let a new call
    site fall back to shared state silently, which is exactly how the trigger
    sites drifted apart from setUPIInput four separate times (INTENT L5).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <map>
#include <vector>

//==============================================================================
struct ProgressiveTransformState
{
    // Keyed by "<base pattern bits><transformer letter><target onsets>", e.g.
    // "10000000e8" for E(1,8)E>8. Per instance now, so the key only has to be
    // unique within one player's own patterns.
    std::map<juce::String, std::vector<bool>> patterns;
    std::map<juce::String, int> stepCount;    // 1-based; step 1 IS the base (INTENT D6)
    std::map<juce::String, int> accessCount;  // LRU bookkeeping for evictIfNeeded()

    /**
     * Cap on distinct keys before least-used entries are dropped. UNCHANGED at
     * 100 from the process-wide version: a per-instance map could justify a much
     * smaller cap, but eviction is observable (a dropped key restarts at its
     * base), so changing it is a separate decision that should be made out loud
     * rather than as a side effect of this one.
     */
    static constexpr int maxStates = 100;

    /** Drop least-used entries once past maxStates. */
    void evictIfNeeded();

    /** Forget one key, so its next parse starts from the base again. */
    void forget (const juce::String& key);

    /** Forget everything this instance knows. */
    void clear();

    /** Which step `key` is on, 1-based. 1 (the base) when the key is unknown. */
    int stepCountFor (const juce::String& key) const;

    /**
     * Persistence, in the shape SceneManager already uses (flat properties on
     * the processor's own state tree, joined with saveStateTo). This is the
     * half of F1 that a per-instance map alone does not fix: the state has to
     * be reachable BY THE PROJECT FILE, or a reloaded session still resumes
     * from whatever the process happens to hold.
     */
    void saveTo (juce::ValueTree& state) const;
    void restoreFrom (const juce::ValueTree& state);
};
