/*
 * RhythmConformanceTests — the plugin's numeric notation cannot drift from the suite.
 *
 * Checks PatternUtils' display codecs (binary / hex / octal / decimal) against
 * kRhythmCodecVectors — the same cross-language vectors the webapp's
 * PatternConverter is tested against (WebApp/tests/rhythm-vectors.json, sourced
 * from @enkerli/theory). Convention: leftmost = LSB, first step = bit 0, so a
 * single onset on step k has value 2^k. See CONVENTIONS.md.
 *
 * This covers the ENCODE / display direction, which is where the historical
 * bit-order discrepancies surfaced. Decode coverage (UPIParser::parseDecimal /
 * parseNumericPattern) is the next increment — it pulls in PatternEngine and
 * QuantizationEngine, so it links the full Core rather than PatternUtils alone.
 *
 * Built by CMake as the `serpe_conformance` target (links juce_core only);
 * `ctest -R rhythm_conformance` runs it, and it is also a post-build step on
 * the Serpe plugin so a normal build surfaces drift.
 */
#include "RhythmConformanceVectors.h"
#include "../Core/PatternUtils.h"
#include <iostream>
#include <string>
#include <vector>

namespace {

std::vector<bool> bits(const char* s) {
    std::vector<bool> p;
    for (const char* c = s; *c; ++c) p.push_back(*c == '1');
    return p;
}

// PatternUtils prefixes: hex "0x", octal "o", decimal "d". Strip to the bare
// numeral the vectors carry. Binary is already bare.
std::string strip(juce::String s, const char* prefix) {
    std::string out = s.toStdString();
    const std::string p(prefix);
    if (out.rfind(p, 0) == 0) out = out.substr(p.size());
    return out;
}

int failures = 0;
void expectEq(const std::string& what, const std::string& got, const std::string& want,
              const char* pattern) {
    if (got != want) {
        ++failures;
        std::cout << "  ✗ " << pattern << ": " << what
                  << " got \"" << got << "\", want \"" << want << "\"\n";
    }
}

} // namespace

int main() {
    std::cout << "=== Rhythm Codec Conformance (encode/display) ===\n";
    std::cout << kRhythmCodecVectorCount << " vectors (leftmost = LSB)\n";

    for (int i = 0; i < kRhythmCodecVectorCount; ++i) {
        const auto& v = kRhythmCodecVectors[i];
        const auto p = bits(v.pattern);

        expectEq("binary",  PatternUtils::getBinaryString(p).toStdString(), v.pattern, v.pattern);
        expectEq("hex",     strip(PatternUtils::getHexString(p), "0x"),     v.hex,     v.pattern);
        expectEq("octal",   strip(PatternUtils::getOctalString(p), "o"),    v.octal,   v.pattern);
        expectEq("decimal", strip(PatternUtils::getDecimalString(p), "d"),  std::to_string(v.decimal), v.pattern);
    }

    if (failures > 0) {
        std::cout << "\n❌ " << failures << " mismatch(es) — the plugin has drifted from the suite convention.\n";
        return 1;
    }
    std::cout << "\n✅ All " << kRhythmCodecVectorCount
              << " vectors match. Plugin display codecs agree with the webapp and @enkerli/theory.\n";
    return 0;
}
