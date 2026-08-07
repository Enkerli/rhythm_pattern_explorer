/*
    ParserProbe — print what UPIParser actually produces for a list of inputs.

    Written after a round of "weird result" reports that could not be
    diagnosed remotely (D:2,3 ...- → 0011110; L:2,3 ...-. → 3 empty steps).
    Guessing at a parser from the outside is slow and unreliable; this makes
    it answer for itself, and it runs in a second with no DAW.

    Build+run:
      cmake --build build --target serpe_parser_probe
      ./build/serpe_parser_probe_artefacts/Release/serpe_parser_probe
*/
#include "UPIParser.h"
#include "PatternEngine.h"
#include "PatternUtils.h"
#include <cstdio>
#include <string>
#include <vector>

static void probe (const char* input)
{
    // A throwaway state per probe: these are one-off parses, and giving each
    // its own is now the only way to be sure one line cannot affect the next.
    ProgressiveTransformState progressive;
    auto r = UPIParser::parse (juce::String (input), progressive);
    std::string bits;
    for (bool b : r.pattern) bits += (b ? '1' : '0');

    std::printf ("%-30s ", input);
    if (! r.isValid())
    {
        std::printf ("ERROR: %s\n", r.errorMessage.toRawUTF8());
        return;
    }
    int onsets = 0;
    for (bool b : r.pattern) if (b) ++onsets;
    std::printf ("%-14s %2d steps %2d onsets", bits.c_str(), (int) r.pattern.size(), onsets);
    if (r.hasMicrotiming) std::printf ("  PD(depth=%.2f seed=%d)", r.microtimingDepth, r.microtimingSeed);
    if (r.hasLongShort)   std::printf ("  LS(%.2f..%.2f d=%.2f)", r.longShortMin, r.longShortMax, r.longShortDepth);
    std::printf ("  name=\"%s\"\n", r.patternName.toRawUTF8());
}

// PROGRESSIVE NOTATION COMES IN TWO KINDS, AND THEY LIVE IN DIFFERENT LAYERS.
// Getting that wrong is what made three rows of this probe meaningless.
//
//   `pat>N`, `pat*N`  — LENGTHENING / TRANSFORM. Stateful IN THE PARSER:
//                       ProgressiveTransformState holds the grown pattern and
//                       each parse() of the same string returns the next step.
//                       probeProgressive below is the right instrument.
//
//   `pat%N`, `pat+N`  — PROGRESSIVE OFFSET. NOT applied by the parser at all.
//                       UPIParser returns the BARE BASE plus the step size
//                       (initialOffset / progressiveOffset) and leaves the
//                       rotation to the caller: PatternEngine tracks the
//                       offset, the processor rotates by -offset (negative =
//                       clockwise). Use probeProgressiveOffset.
//
// Running the second kind through the first printed the SAME pattern on every
// trigger — three rows that read as reference vectors and could never match a
// port that does advance. A JS/C++ comparison against them would show three
// "divergences" that are really one instrument reading the wrong layer, and
// the honest risk is that someone reconciles them by breaking the JS.

// Lengthening/transform: one state across the whole sequence, which is what
// makes this a sequence rather than `triggers` identical parses. Per CALL, so
// two entries with the same pattern text cannot contaminate each other as they
// did while the state was process-wide (F1).
static void probeProgressive (const char* input, int triggers)
{
    std::printf ("%-22s ", input);
    // ONE state across the whole sequence — that is what makes this a sequence
    // rather than `triggers` identical parses. It is also per CALL now, so two
    // entries with the same pattern text no longer contaminate each other, as
    // they did while the state was process-wide (F1).
    ProgressiveTransformState progressive;
    for (int i = 0; i < triggers; ++i)
    {
        auto r = UPIParser::parse (juce::String (input), progressive);
        if (! r.isValid()) { std::printf (" ERROR"); break; }
        std::string bits;
        for (bool b : r.pattern) bits += (b ? '1' : '0');
        std::printf ("%s%s", (i ? " " : ""), bits.c_str());
    }
    std::printf ("\n");
}

// Progressive OFFSET, driven the way the plugin drives it: the real
// bookkeeping (PatternEngine::triggerProgressiveOffset / getCurrentOffset) and
// the real rotation (PatternUtils::rotatePattern), rather than a second copy of
// either. Rotates from the freshly parsed BASE every trigger — the poly path's
// approach — so the offsets cannot compound; the mono path rotates the already
// rotated pattern by -N each time, which for a pure rotation is the same
// sequence.
//
// PHASE, per INTENT D6 and PatternEngine.cpp: trigger 1 is the BARE BASE. So
// the offset is stepped AFTER each print, not before.
static void probeProgressiveOffset (const char* input, int triggers)
{
    std::printf ("%-22s ", input);
    ProgressiveTransformState progressive;
    auto base = UPIParser::parse (juce::String (input), progressive);
    if (! base.isValid())            { std::printf (" ERROR: %s\n", base.errorMessage.toRawUTF8()); return; }
    if (! base.hasProgressiveOffset) { std::printf (" NOT a progressive offset\n"); return; }

    PatternEngine engine;
    engine.setProgressiveOffset (true, base.initialOffset, base.progressiveOffset);
    for (int i = 0; i < triggers; ++i)
    {
        // Negative rotation for clockwise progression, as both paths do.
        auto rotated = PatternUtils::rotatePattern (base.pattern, -engine.getCurrentOffset());
        std::string bits;
        for (bool b : rotated) bits += (b ? '1' : '0');
        std::printf ("%s%s", (i ? " " : ""), bits.c_str());
        engine.triggerProgressiveOffset();
    }
    std::printf ("\n");
}

int main()
{
    std::printf ("=== UPIParser probe — what the C++ engine actually returns ===\n\n");
    // NOTE ON VALUES: E(3,8)/tresillo/0x94 appear all over this codebase, so a
    // parser can look correct on them through shorthand tables and special
    // cases alone. Most inputs below deliberately use unfamiliar numbers
    // (E(5,13), P(7,2), 0x2E, d201) that nothing can have memorised; the few
    // canonical ones are kept where the point IS the canonical result.
    const std::vector<const char*> inputs {
        // Baselines
        "E(3,8)", "E(5,13)", "E(7,16,3)", "M:CQ", "..-.",
        // The aksak notation
        "A(2,2,2,3)", "A(3,3,2)", "A(4,3,2,2)",
        // Custom durations, both spellings, and the trailing-dot case
        "L:2,3 ...-", "D:2,3 ...-", "L:2,3 ...-.", "D:2,3 ...-.",
        "L:1,5 .-", "D:3,1 .-",
        // Feel suffixes, alone and attached
        "E(5,13) PD(50%)", "E(5,13) PD(90%)", "A(3,3,2) PD(60%)",
        "E(5,13) LS(0.5)", "E(5,13) LS(1.4..1.8, 70%)",
        "E(5,13) LS(1.4..1.8) PD(30%)",
        // Things that must NOT be mistaken for the above. The generators B/W/D
        // share letters with Morse; `d…` is also the decimal prefix (that
        // collision is what ate "D:2,3 ...-"); and '+'/'-' between whole
        // patterns must still combine even though '-' is a Morse dash.
        "PD(50%)", "B(5,13)", "W(5,13)", "D(5,13)",
        "E(5,13)+P(7,2)", "E(7,16)-E(3,16)", "1010-0011",
        "E(5,13)+P(7,2) PD(40%)",
        // Step counts are exactly what the notation asks for — no 8-step
        // floor. Padding is `:N`'s job.
        "d201", "d201:12", "0x2E", "0x2E:9", "b1011", "o17", "[0,2]", "[0,2]:8",
        // Polygons in a combination: shapes projected onto the shared cycle,
        // never tiled. E(3,8)+P(4,0) is the v0.02a README's own example.
        "P(7,2)", "P(4,0)", "E(3,8)+P(4,0)", "P(3,0)+P(5,0)", "E(3,8)+P(3,0)",
        "P(3,1)+P(5,0)+P(2,5)", "P(3,0)+P(5,1)-P(2,0)", "E(5,13)+E(2,4)",
        // Third polygon argument is an EXPANSION FACTOR (k*x steps), per the
        // original webapp's "Triangle x4" help text.
        "P(3,1,4)", "P(7,2,2)",
        // `;N` — Lascabettes angular quantization. Re-grids ANY pattern onto N
        // steps by onset angle; `;-N` goes counter-clockwise. This is the
        // arbitrary-step-count operator, which is why P()'s third argument is
        // free to mean expansion. Upward re-grids are lossless; downward ones
        // MERGE collisions, so onset counts can drop.
        // E(3,8);5 is the direction check: its onsets are asymmetric, so
        // clockwise and counter-clockwise genuinely differ. Symmetric
        // sources (E(5,13), an all-onset polygon) give the same set both
        // ways and would hide a broken sign.
        "E(3,8);5", "E(3,8);-5", "E(5,13);8", "E(5,13);-8", "P(7,2);12",
        "E(5,13);16", "E(5,13);4", "E(3,8);3",
        "0x2E:9;6", "A(3,3,2);12", "M:CQ;10",
        // …and it composes with combination and the feel suffixes.
        "P(3,0)+P(5,0);16", "E(5,13);8 PD(30%)", "E(5,13);8 LS(2)",
        "0x1", "0x10", "o10", "d5",
    };
    for (auto* in : inputs) probe (in);

    // ── progressive: one line per input, `triggers` successive results ──────
    std::printf ("\n=== progressive (stateful: each column is the next trigger) ===\n\n");
    probeProgressive ("E(1,8)>8", 10);
    probeProgressive ("B(1,17)>17", 8);
    probeProgressive ("E(8,8)>1", 9);
    probeProgressive ("W(1,13)>13", 6);
    probeProgressive ("D(1,9)>9", 6);

    // Progressive OFFSET — a different layer, so a different instrument.
    std::printf ("\n=== progressive offset (rotation; parser returns the base, engine rotates) ===\n\n");
    probeProgressiveOffset ("E(3,8)%2", 6);
    probeProgressiveOffset ("E(3,8)+3", 5);
    probeProgressiveOffset ("E(5,13)%5", 5);
    probeProgressiveOffset ("E(7,16)%3", 5);
    probeProgressiveOffset ("P(3,1,4)%2", 6);  // a polygon rotates like anything else

    std::printf ("\n(compare against the webapp / `msuite upi` for the same strings)\n");
    return 0;
}
