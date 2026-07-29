/*
    Poly precedence: '/' binds loosest (Alex, 2026-07-28).

    A top-level '/' means the string is parallel lanes, and scenes '|' and
    progressive '%N' '*N' '>N' belong to a LANE rather than to the whole
    string. Before this was settled the two engines read the same text
    differently — the JS parser split on '/' first, the C++ peeled scenes and
    progressive off first — so `E(3,8)%2|E(3,8)*3/E(3,7)` lost a whole lane
    without complaining.

    These tests pin the split and the two guards that stopped the silent loss.
*/
#include <JuceHeader.h>
#include "../Core/PolyParser.h"
#include "../Managers/SceneManager.h"
#include <cstdio>

static int failures = 0;

static void expect (bool ok, const juce::String& what)
{
    std::printf ("  %s %s\n", ok ? "ok  " : "FAIL", what.toRawUTF8());
    if (! ok) ++failures;
}

static void expectLanes (const char* input, int expected)
{
    const int got = PolyParser::splitLanes (juce::String (input)).size();
    expect (got == expected,
            juce::String (input) + " -> " + juce::String (got) + " lane(s), expected " + juce::String (expected));
}

/** What SceneManager makes of a single scene string. */
static void expectScene (const char* input, const char* base, int offset, int lengthening)
{
    SceneManager sm;
    sm.initializeScenes (juce::StringArray::fromTokens (juce::String (input), "|", ""));
    const auto gotBase = sm.getCurrentSceneBasePattern();
    const int gotOff = sm.getCurrentSceneProgressiveOffset();
    const int gotLen = sm.getCurrentSceneProgressiveLengthening();
    expect (gotBase == juce::String (base) && gotOff == offset && gotLen == lengthening,
            juce::String (input) + " -> base=[" + gotBase + "] offset=" + juce::String (gotOff)
              + " lengthening=" + juce::String (gotLen)
              + "   (expected [" + base + "] " + juce::String (offset) + " " + juce::String (lengthening) + ")");
}

int main()
{
    std::printf ("=== poly '/' binds loosest ===\n");
    expectLanes ("E(3,8)", 1);
    expectLanes ("E(3,17)/E(3,5)", 2);
    // The three Alex reported. Each is lanes FIRST; what is left inside a lane
    // is that lane's business.
    expectLanes ("E(3,17)%2/E(3,5)|E(3,8)*3", 2);
    expectLanes ("E(3,17)/E(3,5)%2", 2);
    expectLanes ("E(3,8)%2|E(3,8)*3/E(3,7)", 2);

    std::printf ("\n=== a progressive tail must be a bare number ===\n");
    // The regression that lost a lane: "3/E(3,7)" is not a lengthening amount,
    // and getIntValue() must not be allowed to decide that it is.
    expectScene ("E(3,8)*3/E(3,7)", "E(3,8)*3/E(3,7)", 0, 0);
    expectScene ("E(3,17)%2/E(3,5)", "E(3,17)%2/E(3,5)", 0, 0);
    // Plain forms keep working exactly as before.
    expectScene ("E(3,8)*3", "E(3,8)", 0, 3);
    expectScene ("E(3,8)%2", "E(3,8)", 2, 0);
    expectScene ("E(3,8)", "E(3,8)", 0, 0);

    std::printf ("\n%s — %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
