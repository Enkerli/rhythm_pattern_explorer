/*
    Microtiming conformance — locks Source/Core/Microtiming.h to the webapp's
    packages/upi/src/microtiming.js, vector for vector.

    Why vectors and not just "does it look right": push/pull is FELT, and a
    plugin that leans differently from the browser version of the same pattern
    is a bug nobody can describe. The generator lives in the JS; this proves
    the C++ reproduces it exactly, including the RNG.

    Build+run:  cmake --build build --target serpe_microtiming_conformance
                ./build/serpe_microtiming_conformance_artefacts/serpe_microtiming_conformance
*/
#include "Microtiming.h"
#include "MicrotimingVectors.h"
#include <cmath>
#include <cstdio>
#include <string>

namespace {
int failures = 0, checks = 0;

void expectNear (const std::string& what, double got, double want, double tol = 1e-9)
{
    ++checks;
    if (std::fabs (got - want) > tol)
    {
        std::printf ("FAIL %s: got %.9f want %.9f\n", what.c_str(), got, want);
        ++failures;
    }
}
} // namespace

int main()
{
    std::printf ("=== Microtiming Conformance (parity with microtiming.js) ===\n");

    for (const auto& v : kMicrotimingVectors)
    {
        std::vector<bool> steps;
        for (const char* p = v.binary; *p != '\0'; ++p) steps.push_back (*p == '1');

        std::vector<double> shift, scales;
        serpe::microtiming::microtiming (steps, v.depth, v.seed, v.pass, shift);
        serpe::microtiming::timingScales (shift, scales);

        const std::string tag = std::string (v.binary) + " d=" + std::to_string (v.depth)
                              + " seed=" + std::to_string (v.seed) + " pass=" + std::to_string (v.pass);

        if (shift.size() != v.shift.size()) { std::printf ("FAIL %s: shift size\n", tag.c_str()); ++failures; continue; }
        for (size_t i = 0; i < v.shift.size(); ++i)
            expectNear (tag + " shift[" + std::to_string (i) + "]", shift[i], v.shift[i]);
        for (size_t i = 0; i < v.scales.size(); ++i)
            expectNear (tag + " scale[" + std::to_string (i) + "]", scales[i], v.scales[i]);

        // The invariant that matters most on the audio thread: the bar keeps
        // its length, so the pattern can never walk away from the host clock.
        double total = 0.0;
        for (double s : scales) total += s;
        expectNear (tag + " bar length", total, static_cast<double> (steps.size()));
    }

    // Depth 0 must be dead straight — the "off" switch has to be exact.
    {
        std::vector<bool> steps { true, false, true, false, true, false, true, false, false };
        std::vector<double> shift, scales;
        serpe::microtiming::microtiming (steps, 0.0, 1, 0, shift);
        serpe::microtiming::timingScales (shift, scales);
        for (size_t i = 0; i < shift.size(); ++i) expectNear ("depth0 shift", shift[i], 0.0);
        for (size_t i = 0; i < scales.size(); ++i) expectNear ("depth0 scale", scales[i], 1.0);
    }

    std::printf ("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
