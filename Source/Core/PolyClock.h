/*
  ==============================================================================

    PolyClock.h
    Pure scheduling math for poly-lane playback (music-suite docs/
    SERPE_POLY.md §3b "cycle lock", §8.1 milestone-2 draft). No JUCE
    dependency, no side effects — the whole point is that this is testable
    without an AudioProcessor, a host, or a real device.

    Cycle-lock model: every lane spans the SAME shared cycle (in beats); a
    lane's own step duration = cycleLengthInBeats / itsOwnStepCount. That's
    what makes a 3-step lane against a 16-step lane a true cross-rhythm
    (POLYRHYTHM) rather than two clocks drifting toward a shared lcm
    (POLYMETER) — the field-tested webapp default, ported as-is.

  ==============================================================================
*/

#pragma once

#include <cmath>

//==============================================================================
struct PolyStepResult
{
    bool crossed = false;        // did this lane cross into a new step since lastProcessedStep?
    int step = 0;                 // the step index it crossed into, if crossed
    double fractionalPos = 0.0;   // 0..1, this crossing's position within the current audio buffer
};

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
