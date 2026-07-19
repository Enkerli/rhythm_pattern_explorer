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

    if (failures > 0) {
        std::cout << "\nFAIL: " << failures << " mismatch(es) — the plugin's poly parser has drifted from the webapp.\n";
        return 1;
    }
    std::cout << "\nOK: all " << kPolyConformanceVectors.size()
              << " vectors match. Plugin poly-lane parsing agrees with the webapp.\n";
    return 0;
}
