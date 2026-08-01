/*
  ==============================================================================

    PolyClock.h
    Pure scheduling math for poly-lane playback (music-suite docs/
    SERPE_POLY.md §3b — both lock modes, §8.1 milestone-2 draft +
    milestone-4 polymeter). No JUCE dependency, no side effects — the
    whole point is that this is testable without an AudioProcessor, a
    host, or a real device.

    Two lock modes, both real here (2026-07-20 — step lock was cycle-lock
    only for a while; the webapp's own scheduler always had both, this
    caught up to it):

    computePolyLaneStep (cycle lock, the default): every lane spans the
    SAME shared cycle (in beats); a lane's own step duration =
    cycleLengthInBeats / itsOwnStepCount. That's what makes a 3-step lane
    against a 16-step lane a true cross-rhythm (POLYRHYTHM) — the
    field-tested webapp default, ported as-is.

    computePolyLaneStepPolymeter (step lock): every lane's STEP is the
    same duration instead — lanes of different lengths take different
    total times to complete a cycle and drift out of phase, realigning
    only at the lcm of their lengths (POLYMETER).

  ==============================================================================
*/

#pragma once

#include <cmath>
#include <vector>

//==============================================================================
struct PolyStepResult
{
    bool crossed = false;        // did this lane cross into a new step since lastProcessedStep?
    int step = 0;                 // the step index it crossed into, if crossed
    double fractionalPos = 0.0;   // 0..1, this crossing's position within the current audio buffer
    // The lane's continuous position within its own cycle (0 .. stepCount),
    // always set — `crossed`/`step` are the NOMINAL grid, and a caller applying
    // microtiming (PD) needs the raw position to re-derive the boundary itself.
    double posInCycle = 0.0;
    // The same position measured from the START of the transport, in this
    // lane's own steps, and NOT wrapped — so a caller can tell which cycle the
    // lane is on. Accents need that: the layer is indexed by cumulative onset,
    // so cycle 2's first onset must not read as cycle 1's.
    double posFromStart = 0.0;
};

/**
 * How many onsets this lane has played BEFORE the step it is now on, counted
 * from the start of the transport.
 *
 * Derived, never accumulated (CLAUDE.md, "derived indices"): full cycles come
 * from @p posFromStart, the partial cycle from the pattern itself, so a lane
 * cannot drift out of step with its own accent layer however long it runs.
 * This is the poly twin of the mono getCurrentOnsetCount(), which does the same
 * arithmetic against transportTick.
 *
 * @p stepInCycle is the step about to sound — the onset ON it is not counted,
 * so the very first onset of a run gets accent index 0, as mono does. It may
 * differ from the nominal grid position by one step when the caller applies
 * microtiming (PD), which is reconciled below.
 */
inline long long polyLaneOnsetIndex (const std::vector<bool>& pattern, int stepInCycle,
                                     double posFromStart) noexcept
{
    const int n = static_cast<int> (pattern.size());
    if (n <= 0) return 0;

    int onsetsPerCycle = 0;
    for (bool b : pattern) if (b) ++onsetsPerCycle;
    if (onsetsPerCycle == 0) return 0;

    long long cycle = static_cast<long long> (std::floor (posFromStart / static_cast<double> (n)));

    // PD displaces the step BOUNDARY, so a lane can be sounding step 0 of the
    // next cycle while the nominal clock is still in this one (or the reverse,
    // holding the last step past the line). The displacement is bounded well
    // under one step, so a gap of more than half the pattern can only mean the
    // wrap: without this the onset index jumps by a whole cycle's worth for one
    // onset, and that one onset takes the wrong accent.
    double inCycle = std::fmod (posFromStart, static_cast<double> (n));
    if (inCycle < 0.0) inCycle += n;
    const int nominal = static_cast<int> (inCycle);
    if (nominal - stepInCycle > n / 2)      ++cycle;   // displaced forward, over the line
    else if (stepInCycle - nominal > n / 2) --cycle;   // held back, still in the old cycle

    if (cycle < 0) cycle = 0; // pre-roll: a negative ppq is still cycle 0

    int before = 0;
    for (int i = 0; i < stepInCycle && i < n; ++i)
        if (pattern[static_cast<size_t> (i)]) ++before;

    return cycle * onsetsPerCycle + before;
}

/**
 * Where is this lane's clock right now, and did it just cross a step
 * boundary? ppqPosition is the host's current beat position (monotonic
 * within a cycle; a stopped/rewound transport simply re-derives from the
 * new position, no drift possible since nothing is accumulated across
 * calls except lastProcessedStep for edge detection).
 */
inline PolyStepResult computePolyLaneStep(double ppqPosition, double cycleLengthInBeats,
                                           int laneStepCount, int lastProcessedStep)
{
    PolyStepResult r;
    if (laneStepCount <= 0 || cycleLengthInBeats <= 0.0) return r;

    double beatsPerLaneStep = cycleLengthInBeats / laneStepCount;
    double stepsFromStart = ppqPosition / beatsPerLaneStep;
    double stepsInCurrentCycle = std::fmod(stepsFromStart, static_cast<double>(laneStepCount));
    if (stepsInCurrentCycle < 0.0) stepsInCurrentCycle += laneStepCount; // fmod can be negative pre-roll

    r.posInCycle = stepsInCurrentCycle;
    r.posFromStart = stepsFromStart;

    int currentStep = static_cast<int>(stepsInCurrentCycle);
    if (currentStep != lastProcessedStep)
    {
        r.crossed = true;
        r.step = currentStep;
        r.fractionalPos = std::fmod(stepsInCurrentCycle, 1.0);
    }
    return r;
}

/**
 * Step-lock (POLYMETER) variant of computePolyLaneStep — music-suite
 * docs/SERPE_POLY.md §3b's "Step lock (toggle) = POLYMETER" mode, ported
 * from the webapp's own scheduler (apps/serpe/engine/poly-clock.js:
 * laneStepMs's `polyLock === "step"` branch, unit-tested there with
 * coprime step counts so the drift-and-realign math is provably right,
 * not just assumed).
 *
 * Where computePolyLaneStep (cycle lock) has every lane span the SAME
 * cycle — step duration scales with the lane's own step count — this has
 * every lane's STEP be the same duration (@p baseStepBeats, the mono
 * grid's own subdivision rate, shared across all lanes). Lanes of
 * different lengths therefore take different total times to complete a
 * cycle and drift out of phase, realigning only at the lcm of their
 * lengths — the actual mathematical definition of polymeter, not merely a
 * differently-labeled cycle lock.
 *
 * Deliberately a separate function rather than a lock-mode branch inside
 * computePolyLaneStep: the existing conformance tests pin that function's
 * exact behavior, and there is no DAW in this environment to re-verify a
 * refactor against — additive is lower-risk than reshaping code that
 * already has hand-verified, unrepeatable-here coverage.
 */
inline PolyStepResult computePolyLaneStepPolymeter(double ppqPosition, double baseStepBeats,
                                                     int laneStepCount, int lastProcessedStep) noexcept
{
    PolyStepResult r;
    if (laneStepCount <= 0 || baseStepBeats <= 0.0) return r;

    double stepsFromStart = ppqPosition / baseStepBeats;
    double stepsInCurrentCycle = std::fmod(stepsFromStart, static_cast<double>(laneStepCount));
    if (stepsInCurrentCycle < 0.0) stepsInCurrentCycle += laneStepCount; // fmod can be negative pre-roll

    r.posInCycle = stepsInCurrentCycle;
    r.posFromStart = stepsFromStart;

    int currentStep = static_cast<int>(stepsInCurrentCycle);
    if (currentStep != lastProcessedStep)
    {
        r.crossed = true;
        r.step = currentStep;
        r.fractionalPos = std::fmod(stepsInCurrentCycle, 1.0);
    }
    return r;
}

/**
 * A lane's Keil micro-timing offset (ms, or a tempo-synced note-value
 * fraction — docs/SERPE_POLY.md §2.3) plus the shared base lag, resolved to
 * a sample delay. The lag exists so a negative (push-early) offset has
 * headroom to land before this onset's own base position; the result is
 * never negative — an offset can only use up the lag, not go further back.
 */
inline int computePolyOffsetSamples(bool isFrac, int fracNum, int fracDen, double offsetMs,
                                     double lagMs, double bpm, double sampleRate)
{
    double resolvedOffsetMs = offsetMs;
    if (isFrac && fracDen != 0)
    {
        double msPerBeat = bpm > 0.0 ? (60000.0 / bpm) : 500.0;
        double wholeNoteMs = msPerBeat * 4.0; // a whole note = 4 beats
        resolvedOffsetMs = wholeNoteMs * (static_cast<double>(fracNum) / static_cast<double>(fracDen));
    }
    double totalMs = lagMs + resolvedOffsetMs;
    if (totalMs < 0.0) totalMs = 0.0;
    // Round to nearest, not truncate: a value like 71.99999999997ms (the
    // floating-point result of 60+12) must round to 3456 samples at 48kHz,
    // not silently drop a sample to 3455.
    return static_cast<int>(std::lround((totalMs / 1000.0) * sampleRate));
}
