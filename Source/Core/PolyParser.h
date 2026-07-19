/*
  ==============================================================================

    PolyParser.h
    Poly-lane notation: parallel lanes on top of UPIParser (docs/SERPE_POLY.md
    in music-suite, notation DECIDED 2026-07-18). Ported from the reference
    JS (music-suite packages/upi/src/poly.js) and conformance-locked against
    the same vectors (Source/Tests/PolyConformanceVectors.h).

    `/` separates lanes, `name=` labels one, `@` carries the per-lane
    micro-timing offset (Keil's participatory discrepancies):

      kick=E(4,16) / snare=E(2,4)@+12ms / hat={10}E(8,16)@-1/64

    Two offset units: `@±N[ms]` = absolute milliseconds (clamped +-50ms);
    `@±num/den` = a note-value fraction of a whole note, tempo-synced
    (clamped +-1/8). The splitter consumes each `@` token ATOMICALLY, so a
    fraction's slash never reads as a lane break.

    This sits BESIDE UPIParser, not inside it: a single lane with no `/`
    delegates straight to UPIParser::parse, so mono patterns are unaffected.
    Sound routing (note/channel/mute) stays out of the notation on
    principle: the notation says WHEN, the instrument rack says WHAT.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
struct PolyOffset
{
    enum Kind { None, Ms, Frac };
    Kind kind = None;
    int ms = 0;    // Kind::Ms
    int num = 0;   // Kind::Frac (signed)
    int den = 1;   // Kind::Frac

    bool operator== (const PolyOffset& o) const
    {
        if (kind != o.kind) return false;
        if (kind == Ms) return ms == o.ms;
        if (kind == Frac) return num == o.num && den == o.den;
        return true;
    }
};

struct PolyLane
{
    juce::String label;
    std::vector<bool> steps;
    PolyOffset offset;
    juce::String source;   // lane body after stripping label + offset
};

struct PolyParseResult
{
    bool ok = false;
    std::vector<PolyLane> lanes;
    int lcm = 0;
    juce::String error;
};

//==============================================================================
class PolyParser
{
public:
    static PolyParseResult parse(const juce::String& input);

    /**
     * Split poly notation into lane strings on TOP-LEVEL '/', respecting
     * paren/bracket/brace depth and consuming '@...' offset tokens
     * atomically (so `@+1/32` never splits). Exposed for conformance tests
     * — mirrors splitLanes() in the JS reference exactly.
     */
    static juce::StringArray splitLanes(const juce::String& input);

private:
    struct OffsetParse
    {
        juce::String rest;
        PolyOffset offset;
        bool hasError = false;
        juce::String error;
    };

    // Parse one lane's trailing '@' suffix, if any.
    static OffsetParse parseOffset(const juce::String& laneSrc);

    static int gcdInt(int a, int b);
    static int lcmInt(int a, int b);
};
