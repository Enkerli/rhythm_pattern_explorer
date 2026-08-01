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
#include "ProgressiveTransformState.h"
#include <vector>
#include <functional>

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
    // How many scenes this lane cycles through ('|' inside the lane). 1 when
    // the lane has no chain. The steps/progressive fields below describe the
    // scene that was asked for via parse()'s sceneIndices.
    int sceneCount = 1;
    int sceneIndex = 0;
    std::vector<bool> steps;
    PolyOffset offset;         // the Keil micro-timing offset (@ms / @frac) — NOT the below
    juce::String source;       // lane body after stripping label + offset

    // Progressive-offset state (UPIParser's `@initial#step` rotation syntax,
    // e.g. E(3,8)@0#1) — a DIFFERENT "offset" from PolyOffset above, carried
    // straight from this lane's own UPIParser::ParseResult so the caller can
    // configure (or re-configure, on each trigger) this lane's own
    // PatternEngine independently of every other lane's progressive state.
    bool hasProgressiveOffset = false;
    int progressiveInitialOffset = 0;
    int progressiveOffsetStep = 0;

    // Progressive LENGTHENING (`body*N`) — this lane grows by N steps per
    // trigger. Mutually exclusive with the offset above, the same way the mono
    // path treats them (offset wins if a lane somehow carries both).
    bool hasProgressiveLengthening = false;
    int progressiveLengtheningStep = 0;

    // Per-lane feel, carried from this lane's own UPIParser::ParseResult.
    // `E(3,8) PD(90%) / E(3,8) PD(10%)` is the whole point of putting PD on a
    // lane: two lanes leaning by DIFFERENT amounts. Dropping it here is why
    // those two lanes stayed audibly locked together.
    bool hasMicrotiming = false;
    double microtimingDepth = 0.0;
    int microtimingSeed = 1;

    // This lane's own accent layer — the `{…}` prefix on the lane body.
    // ACCENTS ARE PER-LANE (INTENT §D8): '/' binds loosest, so a brace belongs
    // to the lane it is written in, exactly as scenes and `%N *N >N` do.
    // `{1001010}E(5,8)/E(1,17)>17` accents lane 1 only; each lane gets its own
    // with `{101}E(3,8)/{11}E(3,7)`. Both splitters have always split on '/'
    // BEFORE any accent parsing, so this is what the grammar already said —
    // dropping the field here is what made poly play flat (F2, 2026-08-01).
    //
    // Like mono, the layer is indexed by ONSET, not by step, so an accent
    // pattern whose length is coprime with the lane's onset count precesses
    // across cycles instead of repeating.
    bool hasAccentPattern = false;
    std::vector<bool> accentPattern;
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
    /** Hands back the `>N` bookkeeping belonging to lane `laneIndex`. */
    using LaneProgressiveState = std::function<ProgressiveTransformState&(int laneIndex)>;

    /**
     * laneState(i) supplies lane i's OWN progressive-transform state, which is
     * what makes two lanes over the same text independent. It is REQUIRED, not
     * defaulted: every lane used to parse against one process-wide map keyed by
     * pattern text, so `E(1,8)>8/E(1,8)>8` advanced a single shared counter once
     * per lane and the two identical lanes came apart on trigger 1 (F1a,
     * measured 2026-08-01). A default argument here would let that come back
     * silently — see ProgressiveTransformState.h.
     *
     * beforeLaneParse(laneIndex), if given, runs right before this lane's
     * body goes through UPIParser::parse — the caller's chance to bind a
     * per-lane PatternEngine via UPIParser::setProgressiveOffsetEngine so
     * `@initial#step` progressive syntax reads/writes THAT lane's own state
     * rather than whatever engine was last bound globally. (That binding is
     * still a process-wide static; it is re-bound before every lane, so it does
     * not carry state between parses the way the maps did.)
     */
    static PolyParseResult parse(const juce::String& input,
                                  const LaneProgressiveState& laneState,
                                  const std::function<void(int)>& beforeLaneParse = {},
                                  const std::vector<int>& sceneIndices = {});

    /**
     * Each lane's scene chain, after the label and '@' offset are stripped.
     * A lane with no '|' yields a one-element array, so callers can treat
     * every lane the same. Lets the caller advance its own per-lane scene
     * state BEFORE parse(), then tell parse() which scene each lane is on.
     */
    static std::vector<juce::StringArray> laneScenes(const juce::String& input);

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
