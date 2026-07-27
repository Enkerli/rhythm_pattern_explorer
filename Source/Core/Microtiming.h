/*
    Microtiming.h — participatory discrepancies (Keil) for the audio thread.

    A direct port of music-suite's packages/upi/src/microtiming.js. The two
    implementations are locked together by generated vectors
    (Source/Tests/MicrotimingVectors.h → serpe_microtiming_conformance), the
    same discipline PolyClock/PolyParser already follow: the webapp and the
    plugin must FEEL identical, and "looks equivalent" is not a proof.

    What this is, and is not:
      - it displaces WHERE an attack lands (push/pull), not how long it lasts;
      - it is not swing (fixed and identical every bar) and not jitter
        (independent per-note noise, which merely sounds sloppy);
      - the walk is correlated — it accumulates and resolves — and is pinned
        hardest where the metre is strongest;
      - BAR LENGTH IS PRESERVED EXACTLY: timingScales() differences the
        per-onset displacements, so lengthening one gap shortens another. The
        pattern leans; it never drifts away from the host.

    Header-only, no JUCE, no allocation in the hot path (callers pass their own
    buffers), so it is safe to call from processBlock.
*/
#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace serpe::microtiming
{

/** mulberry32 — byte-identical to the JS. uint32_t wrap-around is the point;
    the JS relies on Math.imul (32-bit signed multiply) and `>>> 0`, both of
    which are exactly unsigned 32-bit arithmetic here. */
class Rng
{
public:
    Rng (int seed, int pass) noexcept
        : state (static_cast<uint32_t> (static_cast<uint32_t> (seed) * 0x9e3779b1u
                                      + static_cast<uint32_t> (pass) * 0x85ebca6bu))
    {}

    double next() noexcept
    {
        state = state + 0x6d2b79f5u;
        uint32_t t = state;
        t = t ^ (t >> 15);
        t = t * (state | 1u);
        // JS: t ^= t + Math.imul(t ^ (t >>> 7), t | 61)
        uint32_t u = (t ^ (t >> 7)) * (t | 61u);
        t = t ^ (t + u);
        return static_cast<double> (t ^ (t >> 14)) / 4294967296.0;
    }

private:
    uint32_t state;
};

/** How far one onset may be displaced (fractions of a step) and how hard the
    walk pushes at a given depth — the two numbers that set how EXTREME the
    feel gets. Must equal MAX_SHIFT / WALK_SCALE in microtiming.js; the
    conformance vectors fail loudly if they drift apart.

    Raised 2026-07-27 (0.35/0.5 -> 0.45/0.75) after playing it: at depth 0.9
    the old numbers displaced onsets by 0.23 of a step on average and clipped
    35% of them flat against the cap, flattening the walk into a square wave at
    exactly the setting meant to be wildest.

    kMaxShift cannot reach 0.5: that is where an onset arrives at its
    neighbour's nominal position, and where displacedIndex()'s +-1-step
    boundary test stops being well-defined. */
inline constexpr double kMaxShift  = 0.45;
inline constexpr double kWalkScale = 0.75;

/** How strongly a position resists being moved. 1 = downbeat, pinned hardest. */
inline double anchorAt (int pos, int n) noexcept
{
    if (pos == 0)                             return 0.85;
    if (n % 2 == 0 && pos == n / 2)           return 0.55;
    if (n % 4 == 0 && (pos % (n / 4)) == 0)   return 0.40;
    return 0.15;
}

/** Per-STEP displacement, in fractions of a step. + = late, - = early.
    Steps without an onset stay 0 (there is nothing there to displace).
    `out` is resized to steps.size(); pass a reused vector to avoid allocating
    on the audio thread. */
inline void microtiming (const std::vector<bool>& steps,
                         double depth, int seed, int pass,
                         std::vector<double>& out,
                         double maxShift = kMaxShift)
{
    const int n = static_cast<int> (steps.size());
    out.assign (static_cast<size_t> (std::max (0, n)), 0.0);
    if (n <= 0 || depth <= 0.0)
        return;

    int onsetCount = 0;
    for (int i = 0; i < n; ++i)
        if (steps[(size_t) i]) ++onsetCount;
    if (onsetCount < 2)
        return;   // nothing to push against

    Rng rng (seed, pass);
    double drift = 0.0;
    for (int pos = 0; pos < n; ++pos)
    {
        if (! steps[(size_t) pos]) continue;
        const double anchor = anchorAt (pos, n);
        drift = drift * (1.0 - anchor) + (rng.next() * 2.0 - 1.0) * depth * kWalkScale;
        const double d = std::max (-maxShift, std::min (maxShift, drift));
        out[(size_t) pos] = (pos == 0) ? 0.0 : d;  // downbeat is the reference
    }
}

/** Displacements -> per-step length multipliers (1 = unchanged).
    interval[i] = nominal + (shift[i+1] - shift[i]), wrapping at the cycle
    boundary, so the multipliers sum to the nominal bar by construction. */
inline void timingScales (const std::vector<double>& shift, std::vector<double>& out)
{
    const size_t n = shift.size();
    out.assign (n, 1.0);
    if (n == 0) return;
    for (size_t i = 0; i < n; ++i)
    {
        const double next = shift[(i + 1) % n];
        out[i] = std::max (0.25, 1.0 + (next - shift[i]));   // never collapse a step
    }
}

/** Which step has actually arrived, given that step i fires at (i + shift[i])
    rather than at i.

    Offsetting the boundary TEST is what lets a note sit early or late without a
    scheduling queue and without added latency: the step simply becomes current
    a little sooner or later than its nominal position. Displacement is bounded
    well under one step, so only the neighbours are ever in play.

    `pos` is the continuous step position within the cycle (0 .. stepCount).
    Shared by the mono path and every poly lane — one groove model, so a lane
    cannot lean differently from the mono pattern that produced it. */
inline int displacedIndex (double pos, int stepCount, const std::vector<double>& shift) noexcept
{
    if (stepCount <= 0) return 0;
    const int nominal = static_cast<int> (pos) % stepCount;
    if (shift.size() != static_cast<size_t> (stepCount))
        return nominal;

    const int next = (nominal + 1) % stepCount;
    const double base = static_cast<double> (static_cast<int> (pos));

    // Has the NEXT step already arrived early (negative shift)?
    if (pos >= base + 1.0 + shift[(size_t) next])
        return next;
    // Has THIS step arrived yet (positive shift means it hasn't)?
    if (pos < base + shift[(size_t) nominal])
        return (nominal - 1 + stepCount) % stepCount;
    return nominal;
}

} // namespace serpe::microtiming
