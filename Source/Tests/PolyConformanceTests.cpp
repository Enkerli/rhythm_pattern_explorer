/*
 * PolyConformanceTests — the plugin's poly-lane parser cannot drift from
 * the webapp's.
 *
 * Checks PolyParser against kPolyConformanceVectors — the same
 * cross-language vectors the webapp's parsePolyUPI is tested against
 * (WebApp/tests/poly-vectors.json, sourced from music-suite
 * packages/upi/src/poly.js). Scope: lane splitting (top-level '/', atomic
 * '@' offset tokens), labels, per-lane offset (both units + clamps), and
 * the resulting lcm — docs/SERPE_POLY.md §8 milestone 1 in music-suite.
 *
 * Built by CMake as the `serpe_poly_conformance` target (links juce_core
 * only, same shape as serpe_conformance); `ctest -R poly_conformance` runs it.
 */
#include "PolyConformanceVectors.h"
#include "../Core/PolyParser.h"
#include "../Core/PolyClock.h"
#include <iostream>
#include <string>

namespace {

std::string bits(const std::vector<bool>& steps) {
    std::string s;
    for (bool b : steps) s += (b ? '1' : '0');
    return s;
}

int failures = 0;

void expectEq(const char* input, const std::string& what,
              const std::string& got, const std::string& want) {
    if (got != want) {
        ++failures;
        std::cout << "  X " << input << ": " << what
                  << " got \"" << got << "\", want \"" << want << "\"\n";
    }
}

void expectTrue(const char* input, const std::string& what, bool cond) {
    if (!cond) {
        ++failures;
        std::cout << "  X " << input << ": " << what << "\n";
    }
}

/*
 * The scheduling math (SERPE_POLY.md §8 milestone 2) can't be checked
 * against the webapp the way parsing can — there's no shared JS reference
 * for real-time playback — so these are hand-computed cases instead: no
 * DAW/host is available in CI or this environment, so this is the only
 * verification the cycle-lock formula gets before a human tests it in a
 * real host. computePolyLaneStep/computePolyOffsetSamples are pure
 * functions (no JUCE, no AudioProcessor) precisely so this is possible.
 */
void testPolyClock() {
    // Cycle-lock alignment: a 4-step lane and a 3-step lane sharing a
    // 4-beat cycle is the textbook 3-against-4 cross-rhythm. Constructing
    // ppq as cycleLengthInBeats * k / laneStepCount makes step k land
    // exactly (no floating-point drift) — the point of the test is the
    // FORMULA, not floating-point noise.
    {
        const double cycle = 4.0;
        for (int k = 0; k < 4; ++k) {
            double ppq = cycle * k / 4;
            auto r = computePolyLaneStep(ppq, cycle, 4, -1);
            expectTrue("4-step lane, 4-beat cycle", ("step " + std::to_string(k)).c_str(), r.crossed && r.step == k);
        }
        for (int k = 0; k < 3; ++k) {
            double ppq = cycle * k / 3;
            auto r = computePolyLaneStep(ppq, cycle, 3, -1);
            expectTrue("3-step lane, 4-beat cycle", ("step " + std::to_string(k)).c_str(), r.crossed && r.step == k);
        }
    }
    // Same step, no crossing.
    {
        auto r = computePolyLaneStep(1.0, 4.0, 4, 1);
        expectTrue("no re-trigger on same step", "crossed should be false", !r.crossed);
    }
    // Wrap-around: a full cycle later lands back on step 0.
    {
        auto r = computePolyLaneStep(4.0, 4.0, 4, 3);
        expectTrue("cycle wrap-around", "step 0 after a full cycle", r.crossed && r.step == 0);
    }
    // Fractional position within the step (mid-step ppq).
    {
        auto r = computePolyLaneStep(0.5, 4.0, 4, -1);
        expectTrue("fractional position", "step 0",  r.crossed && r.step == 0);
        expectEq("fractional position", "fractionalPos", std::to_string(r.fractionalPos), std::to_string(0.5));
    }

    // Offset scheduling: ms unit, no fraction.
    {
        int samples = computePolyOffsetSamples(false, 0, 1, 12.0, 60.0, 120.0, 48000.0);
        // (60 + 12)ms * 48000/1000 = 3456 samples
        expectEq("offset ms", "samples", std::to_string(samples), std::to_string(3456));
    }
    // Offset scheduling: tempo-synced fraction, negative (push early).
    {
        int samples = computePolyOffsetSamples(true, -1, 64, 0.0, 60.0, 120.0, 48000.0);
        // msPerBeat=500, wholeNote=2000ms, -1/64 of that = -31.25ms; (60-31.25)ms*48 = 1380 samples
        expectEq("offset frac", "samples", std::to_string(samples), std::to_string(1380));
    }
    // Offset scheduling: clamp — an offset that would push before this
    // onset's own base position never goes negative.
    {
        int samples = computePolyOffsetSamples(false, 0, 1, -100.0, 60.0, 120.0, 48000.0);
        expectEq("offset clamp", "samples", std::to_string(samples), std::to_string(0));
    }
}

/*
 * Step-lock (POLYMETER) scheduling — computePolyLaneStepPolymeter, added
 * 2026-07-20 alongside the plugin's polyLock param. Same hand-computed
 * approach as testPolyClock() (no DAW here to check against), but using
 * coprime step counts (7 vs 11, lcm 77) rather than small multiples —
 * a multiples-of-8 example realigns almost immediately and wouldn't prove
 * the drift-then-realign behavior that's the actual point of polymeter.
 * Mirrors apps/serpe/engine/poly-clock.test.js in music-suite, the JS
 * reference this was ported from.
 */
void testPolyClockPolymeter() {
    const double baseStepBeats = 0.25; // one 16th note, shared by every lane

    // Both lanes start together at ppq 0.
    {
        auto a = computePolyLaneStepPolymeter(0.0, baseStepBeats, 7, -1);
        auto b = computePolyLaneStepPolymeter(0.0, baseStepBeats, 11, -1);
        expectTrue("7-step lane @ ppq 0", "step 0", a.crossed && a.step == 0);
        expectTrue("11-step lane @ ppq 0", "step 0", b.crossed && b.step == 0);
    }
    // Same step duration for every lane: after 7 steps, the 7-step lane has
    // wrapped back to 0 while the 11-step lane has only reached step 7 —
    // this IS polymeter (step lock), the opposite of cycle lock where both
    // would still be "aligned" at a shared cycle boundary.
    {
        double ppq = baseStepBeats * 7;
        auto a = computePolyLaneStepPolymeter(ppq, baseStepBeats, 7, -1);
        auto b = computePolyLaneStepPolymeter(ppq, baseStepBeats, 11, -1);
        expectTrue("7-step lane after 7 steps", "wraps to step 0", a.crossed && a.step == 0);
        expectTrue("11-step lane after 7 steps", "reaches step 7, no wrap", b.crossed && b.step == 7);
    }
    // The two lanes only realign at the lcm of their lengths (77 steps).
    {
        double ppq = baseStepBeats * 77;
        auto a = computePolyLaneStepPolymeter(ppq, baseStepBeats, 7, -1);
        auto b = computePolyLaneStepPolymeter(ppq, baseStepBeats, 11, -1);
        expectTrue("7-step lane after 77 steps (lcm)", "step 0", a.crossed && a.step == 0);
        expectTrue("11-step lane after 77 steps (lcm)", "step 0", b.crossed && b.step == 0);
    }
    // Same step, no crossing (edge-detection still works for the new fn).
    {
        auto r = computePolyLaneStepPolymeter(0.0, baseStepBeats, 7, 0);
        expectTrue("polymeter no re-trigger on same step", "crossed should be false", !r.crossed);
    }
    // Fractional position within the step (mid-step ppq).
    {
        auto r = computePolyLaneStepPolymeter(baseStepBeats * 0.5, baseStepBeats, 7, -1);
        expectTrue("polymeter fractional position", "step 0", r.crossed && r.step == 0);
        expectEq("polymeter fractional position", "fractionalPos", std::to_string(r.fractionalPos), std::to_string(0.5));
    }
}

} // namespace

int main() {
    std::cout << "=== Poly-lane Conformance ===\n";
    std::cout << kPolyConformanceVectors.size() << " vectors (leftmost = LSB)\n";

    for (const auto& v : kPolyConformanceVectors) {
        auto result = PolyParser::parse(juce::String(v.input));

        expectTrue(v.input, "ok", result.ok == v.ok);
        if (!v.ok) {
            // Error vectors only pin the failure, not the message text —
            // wording naturally differs across languages.
            continue;
        }
        if (!result.ok) continue; // already flagged above; avoid cascading noise

        expectTrue(v.input, "lane count", result.lanes.size() == v.lanes.size());
        expectEq(v.input, "lcm", std::to_string(result.lcm), std::to_string(v.lcm));

        for (size_t i = 0; i < v.lanes.size() && i < result.lanes.size(); ++i) {
            const auto& want = v.lanes[i];
            const auto& got = result.lanes[i];
            std::string ctx = std::string(v.input) + " lane " + std::to_string(i);

            expectEq(ctx.c_str(), "label", got.label.toStdString(), want.label);
            expectEq(ctx.c_str(), "steps", bits(got.steps), want.steps);

            if (want.offset.isMs) {
                expectTrue(ctx.c_str(), "offset kind == Ms", got.offset.kind == PolyOffset::Ms);
                expectEq(ctx.c_str(), "offset.ms", std::to_string(got.offset.ms), std::to_string(want.offset.ms));
            } else if (want.offset.isFrac) {
                expectTrue(ctx.c_str(), "offset kind == Frac", got.offset.kind == PolyOffset::Frac);
                expectEq(ctx.c_str(), "offset.num", std::to_string(got.offset.num), std::to_string(want.offset.num));
                expectEq(ctx.c_str(), "offset.den", std::to_string(got.offset.den), std::to_string(want.offset.den));
            } else {
                expectTrue(ctx.c_str(), "offset kind == None", got.offset.kind == PolyOffset::None);
            }
        }
    }

    std::cout << "\n=== Poly-lane Scheduling (hand-computed, no host available) ===\n";
    testPolyClock();

    std::cout << "\n=== Poly-lane Scheduling — step lock / polymeter ===\n";
    testPolyClockPolymeter();

    if (failures > 0) {
        std::cout << "\nFAIL: " << failures << " mismatch(es).\n";
        return 1;
    }
    std::cout << "\nOK: all " << kPolyConformanceVectors.size()
              << " parsing vectors + the scheduling cases match.\n";
    return 0;
}
