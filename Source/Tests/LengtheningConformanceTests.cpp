/**
 * Progressive lengthening (`*N`) — does the engine grow a lane the way the
 * JS reference does?
 *
 * Until 2026-08-02 it could not: the engine used an unseeded std::mt19937, so
 * the same notation gave different material in the plugin and in the
 * webapp/CLI. Under INTENT D3 the engine wins where it is present, so nothing
 * was broken — but `*N` could not be pinned by a vector, which meant the one
 * progressive form with generated content had no conformance coverage at all.
 *
 * Both sides now seed from the pattern SO FAR (packages/upi/src/rng.js and
 * rhythm.js; PatternUtils::bellCurveRandomSteps). Stateless by design: the next
 * chunk depends only on what is already there, so the engine can keep appending
 * without tracking a trigger ordinal, which is what kept this a local change.
 *
 * The vectors below are the JS reference's own output, generated from
 * @enkerli/upi. What they really pin is the DRAW SEQUENCE — same mulberry32,
 * same FNV-1a seed, same Box-Muller (two draws, u then v), same Fisher-Yates
 * (n-1 draws, high index down). Any of those differing by one draw sends every
 * later chunk somewhere else, which is why std::normal_distribution and
 * std::shuffle could not be used: both are implementation-defined.
 */
#include <JuceHeader.h>
#include <iostream>
#include <string>
#include <vector>
#include "PatternUtils.h"

namespace {

struct Vec { const char* notation; int trigger; const char* expected; };

// Generated from @enkerli/upi — see the header comment.
const Vec kVectors[] = {
    { "E(3,8)*4", 1, "10010010" },
    { "E(3,8)*4", 2, "100100100011" },
    { "E(3,8)*4", 3, "1001001000110110" },
    { "E(3,8)*4", 4, "10010010001101101001" },
    { "E(5,8)*2", 1, "10110110" },
    { "E(5,8)*2", 2, "1011011001" },
    { "E(5,8)*2", 3, "101101100101" },
    { "E(5,8)*2", 4, "10110110010101" },
    { "E(5,8)*2", 5, "1011011001010110" },
    { "100101010*3", 1, "100101010" },
    { "100101010*3", 2, "100101010110" },
    { "100101010*3", 3, "100101010110011" },
    { "100101010*3", 4, "100101010110011100" },
    { "E(1,8)*1", 1, "10000000" },
    { "E(1,8)*1", 2, "100000001" },
    { "E(1,8)*1", 3, "1000000010" },
    { "E(1,8)*1", 4, "10000000100" },
    { "E(1,8)*1", 5, "100000001001" },
    { "E(7,16)*5", 1, "1010100101010010" },
    { "E(7,16)*5", 2, "101010010101001010111" },
    { "E(7,16)*5", 3, "10101001010100101011101010" },
};
const int kVectorCount = static_cast<int> (sizeof (kVectors) / sizeof (kVectors[0]));

std::vector<bool> bits (const std::string& s)
{
    std::vector<bool> p;
    for (char c : s) p.push_back (c == '1');
    return p;
}
std::string str (const std::vector<bool>& p)
{
    std::string s;
    for (bool b : p) s += b ? '1' : '0';
    return s;
}

/** The base of `pattern*N`, and its step. */
bool splitLengthening (const std::string& notation, std::string& base, int& step)
{
    const auto star = notation.rfind ('*');
    if (star == std::string::npos) return false;
    base = notation.substr (0, star);
    step = std::stoi (notation.substr (star + 1));
    return true;
}

int failures = 0;
void expectEq (const std::string& what, const std::string& got, const std::string& want)
{
    if (got == want) return;
    ++failures;
    std::cout << "  MISMATCH " << what << "\n    expected " << want << "\n    got      " << got << "\n";
}

} // namespace

int main()
{
    std::cout << "=== Progressive lengthening conformance (`*N`) ===\n";
    std::cout << kVectorCount << " vectors, from @enkerli/upi (leftmost = LSB)\n\n";

    for (int i = 0; i < kVectorCount; ++i)
    {
        const auto& v = kVectors[i];
        std::string baseStr; int step = 0;
        if (! splitLengthening (v.notation, baseStr, step))
        {
            ++failures;
            std::cout << "  could not read `" << v.notation << "`\n";
            continue;
        }

        // The base must be a literal binary pattern here — the conformance app
        // does not link the parser, and every vector's base is spelled out in
        // the expected string's first `n` characters.
        const std::string expected = v.expected;
        auto pattern = bits (expected.substr (0, expected.size()
            - static_cast<size_t> (step) * static_cast<size_t> (v.trigger - 1)));

        // Grow exactly as the engine does: append a chunk per trigger, each
        // seeded from the pattern accumulated so far.
        for (int t = 1; t < v.trigger; ++t)
        {
            const auto extra = PatternUtils::bellCurveRandomSteps (
                step, PatternUtils::seedFromSteps (pattern, step));
            pattern.insert (pattern.end(), extra.begin(), extra.end());
        }

        expectEq (std::string (v.notation) + " @ trigger " + std::to_string (v.trigger),
                  str (pattern), expected);
    }

    if (failures > 0)
    {
        std::cout << "\n" << failures << " mismatch(es) — the engine grows `*N` differently from the JS reference.\n";
        return 1;
    }
    std::cout << "All " << kVectorCount << " vectors match. The plugin grows a `*N` lane"
              << " exactly as the webapp and the CLI do.\n";
    return 0;
}
