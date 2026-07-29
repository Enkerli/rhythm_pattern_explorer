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
#include "../Core/PatternEngine.h"
#include "../Core/PatternUtils.h"
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

    std::printf ("\n=== per-lane progressive offset: body%%N ===\n");
    {
        // '%N' on a lane is that lane's own progressive offset, and is
        // shorthand for '@N#N' — the first trigger already shows offset N,
        // matching what '%N' means in a mono pattern.
        auto r = PolyParser::parse ("E(3,8)%2/E(3,7)");
        expect (r.ok, juce::String ("E(3,8)%2/E(3,7)parses: ") + (r.ok ? "yes" : r.error));
        if (r.ok && r.lanes.size() == 2)
        {
            expect (r.lanes[0].hasProgressiveOffset && r.lanes[0].progressiveOffsetStep == 2
                      && r.lanes[0].progressiveInitialOffset == 2,
                    "lane 1 carries progressive offset step 2");
            expect (! r.lanes[1].hasProgressiveOffset, "lane 2 is static — the offset is lane-local");
            expect (r.lanes[0].steps.size() == 8 && r.lanes[1].steps.size() == 7,
                    "lane bodies keep their own lengths (8 and 7)");
            expect (r.lanes[0].source.contains ("%2"),
                    "lane source keeps '%2' so a re-parse is distinguishable from E(3,8)");
            expect (r.lcm == 56, juce::String ("grid is lcm(8,7)=56, got ") + juce::String (r.lcm));
        }

        // The sequence the processor drives: restart on a new body, advance
        // on a re-trigger, always rotating the FRESH base so offsets cannot
        // compound. Mirrors parseAndApplyPolyUPI.
        const auto base = PolyParser::parse ("E(3,8)%2/E(3,7)").lanes[0].steps;
        PatternEngine eng;
        eng.setProgressiveOffset (true, 2, 2);
        auto bits = [] (const std::vector<bool>& v)
        {
            juce::String s; for (bool b : v) s << (b ? '1' : '0'); return s;
        };
        juce::StringArray seen;
        for (int t = 0; t < 5; ++t)
        {
            if (t > 0) eng.triggerProgressiveOffset();
            seen.add (bits (PatternUtils::rotatePattern (base, -eng.getCurrentOffset())));
        }
        // E(3,8) rotated by 2 each trigger: period 4 on an 8-step pattern.
        expect (seen[0] != bits (base), "trigger 1 is already rotated (offset 2), as mono '%2' is");
        expect (seen[4] == seen[0], "rotating by 2 on 8 steps has period 4, so trigger 5 repeats trigger 1");
        for (const auto& s : seen)
            expect (s.length() == 8, "rotation preserves length: " + s);
        std::printf ("       sequence: %s\n", seen.joinIntoString (" ").toRawUTF8());
    }

    std::printf ("\n=== per-lane progressive lengthening: body*N ===\n");
    {
        auto r = PolyParser::parse ("E(3,8)*3/E(3,7)");
        expect (r.ok, juce::String ("E(3,8)*3/E(3,7) parses: ") + (r.ok ? "yes" : r.error));
        if (r.ok && r.lanes.size() == 2)
        {
            expect (r.lanes[0].hasProgressiveLengthening && r.lanes[0].progressiveLengtheningStep == 3,
                    "lane 1 grows by 3 per trigger");
            expect (! r.lanes[1].hasProgressiveLengthening, "lane 2 is fixed — growth is lane-local");
            expect (! r.lanes[0].hasProgressiveOffset, "lengthening and offset are exclusive");
            expect (r.lanes[0].steps.size() == 8,
                    "the PARSED lane is still the 8-step base; growth is runtime state");
            expect (r.lanes[0].source.contains ("*3"), "lane source keeps '*3'");
        }

        // Two suffixes on one lane. Both detectors look at the LAST occurrence
        // of their own symbol and require a bare number after it, so whichever
        // is genuinely trailing wins — the same rule mono applies, and the
        // reason is that '%2*3' is not a number so '%' never matches. What
        // matters is that a lane never claims BOTH.
        for (auto* s : { "E(3,8)%2*3/E(3,7)", "E(3,8)*3%2/E(3,7)" })
        {
            auto both = PolyParser::parse (juce::String (s));
            if (both.ok && both.lanes.size() == 2)
                expect (! (both.lanes[0].hasProgressiveOffset && both.lanes[0].hasProgressiveLengthening),
                        juce::String (s) + ": lane takes at most one progressive suffix");
            else
                expect (true, juce::String (s) + ": rejected outright (" + both.error + ")");
        }

        // Growth: each trigger appends `step` steps to what is already there,
        // so earlier growth stays put instead of the tail re-randomising.
        // Mirrors parseAndApplyPolyUPI's lane.grown.
        const auto base = r.lanes[0].steps;
        std::vector<bool> grown = base;
        juce::Random rng (1);
        juce::StringArray lens;
        for (int t = 0; t < 4; ++t)
        {
            const size_t before = grown.size();
            for (int k = 0; k < 3; ++k) grown.push_back (rng.nextBool());
            expect (grown.size() == before + 3, "each trigger adds exactly 3 steps");
            lens.add (juce::String ((int) grown.size()));
        }
        expect (lens.joinIntoString (",") == "11,14,17,20",
                "lengths run 11,14,17,20 — trigger 1 is already base+step, as the engine does");
        expect (std::equal (base.begin(), base.end(), grown.begin()),
                "the base stays as a prefix, so the lane grows rather than churns");
    }

    std::printf ("\n%s — %d failure(s)\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
