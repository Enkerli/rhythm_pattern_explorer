/*
  ==============================================================================

    Rhythm Pattern Explorer - Pattern Engine Implementation

    Holds ONE pattern and the generators that produce it. Deliberately small and
    stateless-ish: the processor owns scheduling, the UPI parser owns notation,
    and this owns "what are the steps". A poly lane owns its own PatternEngine,
    which is why per-lane progressive state works at all.

    Patterns are std::vector<bool>, leftmost = first step = LSB (INTENT D1 in
    the music-suite monorepo). That rule is deliberate and reaches into the hex
    and octal formatters; it is not a bug to be normalised.

  ==============================================================================
*/

#include "PatternEngine.h"
#include "UPIParser.h"
#include "PatternUtils.h"

//==============================================================================
PatternEngine::PatternEngine()
    : randomEngine(std::random_device{}())
{
    // Initialize with a simple pattern
    currentPattern = {true, false, false, true, false, false, true, false};
    
}

PatternEngine::~PatternEngine()
{
}

//==============================================================================
void PatternEngine::generateEuclideanPattern(int onsets, int steps, int offset)
{
    if (steps <= 0 || onsets < 0 || onsets > steps)
    {
        return;
    }
    
    currentPattern = PatternUtils::bjorklundAlgorithm(onsets, steps);
    
    if (offset != 0)
    {
        currentPattern = PatternUtils::rotatePattern(currentPattern, offset);
    }
    
}

/**
    A "polygon" rhythm: place `vertices` points evenly around a circle of
    `steps` and take whichever steps they land nearest. A triangle in 12 gives
    onsets at 0, 4, 8; a pentagon in 16 gives an uneven-but-regular figure.

    Distinct from Euclidean: Euclidean distributes onsets so the GAPS are as
    even as integers allow, whereas this rounds ideal real-valued positions to
    the grid. They agree when the division is exact and diverge otherwise —
    which is the interesting case, and the reason both exist.

    Collisions are not an error. Two vertices can round to the same step (e.g.
    many vertices in few steps), and the pattern simply has fewer onsets than
    vertices. Silently correct rather than silently wrong.
*/
void PatternEngine::generatePolygonPattern(int vertices, int steps, int offset)
{
    if (vertices <= 0 || steps <= 0)
    {
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Generate regular polygon pattern - vertices equally spaced around circle
    for (int i = 0; i < vertices; ++i)
    {
        // Calculate position with floating point for accuracy, then round
        double exactPos = ((double)(i * steps) / vertices) + offset;
        int position = ((int)std::round(exactPos)) % steps;
        if (position < 0) position += steps; // Handle negative modulo
        
        if (position >= 0 && position < steps)
        {
            currentPattern[position] = true;
        }
    }
    
}

void PatternEngine::generateRandomPattern(int onsets, int steps)
{
    if (steps <= 0 || onsets < 0 || onsets > steps)
    {
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // onsets == 0 means "choose for me", NOT "no onsets" — a bell-curve count
    // centred on half the steps. Surprising enough to state plainly: an empty
    // pattern is unreachable through this function by design, since asking for
    // a random rhythm and being handed silence is never what was meant.
    int actualOnsets = onsets;
    if (onsets == 0)
    {
        actualOnsets = bellCurveOnsetCount(steps);
    }
    
    // Uniform over positions (shuffle, then take the first N). The bell curve
    // above governs HOW MANY, this governs WHERE, and keeping them independent
    // is what stops random patterns from inheriting a metric accent.
    std::vector<int> positions;
    for (int i = 0; i < steps; ++i)
    {
        positions.push_back(i);
    }
    
    std::shuffle(positions.begin(), positions.end(), randomEngine);
    
    for (int i = 0; i < actualOnsets && i < positions.size(); ++i)
    {
        currentPattern[positions[i]] = true;
    }
    
}

void PatternEngine::generateBinaryPattern(int binaryValue, int steps)
{
    // 32 is the int width: bit i of `binaryValue` is step i, so beyond 32 steps
    // there are no bits left to read and the request is meaningless rather than
    // merely large. Rejected outright instead of silently truncating.
    if (steps <= 0 || steps > 32)
    {
        return;
    }
    
    currentPattern.clear();
    currentPattern.resize(steps, false);
    
    // Leftmost = LSB (suite-wide): the first step is bit 0, so step i = bit i.
    for (int i = 0; i < steps; ++i)
    {
        currentPattern[i] = (binaryValue & (1 << i)) != 0;
    }
    
}

//==============================================================================
void PatternEngine::setPattern(const std::vector<bool>& pattern)
{
    currentPattern = pattern;
}

int PatternEngine::getOnsetCount() const
{
    int count = 0;
    for (bool step : currentPattern)
    {
        if (step) count++;
    }
    return count;
}


//==============================================================================
juce::String PatternEngine::getPatternDescription() const
{
    int onsets = getOnsetCount();
    int steps = getStepCount();
    
    juce::String description = "Pattern: " + juce::String(onsets) + "/" + juce::String(steps);
    
    return description;
}

juce::String PatternEngine::getBinaryString() const
{
    return PatternUtils::getBinaryString(currentPattern);
}

juce::String PatternEngine::getHexString() const
{
    return PatternUtils::getHexString(currentPattern);
}

juce::String PatternEngine::getOctalString() const
{
    return PatternUtils::getOctalString(currentPattern);
}

juce::String PatternEngine::getDecimalString() const
{
    return PatternUtils::getDecimalString(currentPattern);
}

//==============================================================================
// Pattern utility functions moved to UPIParser for centralization

int PatternEngine::bellCurveOnsetCount(int steps)
{
    return PatternUtils::bellCurveOnsetCount(steps, randomEngine);
}

//==============================================================================
// Progressive Offset Support
//
// `%N` in UPI: each trigger rotates the pattern a further N steps.
//
// PHASE (INTENT D6, settled 2026-07-30): trigger 1 is the BARE BASE. Callers
// pass `initial = 0` and the first triggerProgressiveOffset() takes it to N.
// Until that date the processor passed `initial = N`, so the un-rotated base
// was never heard — inconsistent with `>N`, which always showed it. If you are
// reading this because a pattern seems to start in the wrong place, the phase
// is the convention, not the bug; see docs/PROGRESSIVE_PHASE.md.
//
// DERIVED, NOT ACCUMULATED: currentOffset is recomputed from triggerCount every
// time rather than being incremented in place. That is what lets a scene resume
// exactly where it left off when a chain comes back round to it, and it is why
// the JS reference can be a pure function of the trigger index.

void PatternEngine::setProgressiveOffset(bool enabled, int initial, int progressive)
{
    hasProgressiveOffset = enabled;
    initialOffset = initial;
    progressiveOffset = progressive;
    currentOffset = initial;
    triggerCount = 0;
    
}

void PatternEngine::triggerProgressiveOffset()
{
    if (!hasProgressiveOffset)
        return;
    
    triggerCount++;
    currentOffset = initialOffset + (triggerCount * progressiveOffset);
    
}
