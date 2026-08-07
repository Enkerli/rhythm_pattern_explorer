/**
 * Progressive OFFSET (`%N`, `+N`) — does the engine rotate a pattern the way
 * the JS reference does?
 *
 * WHY THIS EXISTS. ParserProbe printed these three notations by calling
 * UPIParser::parse() repeatedly, the way it prints `>N` and `*N`. That works
 * for those two, which are stateful IN THE PARSER. It cannot work here: `%N`
 * and `+N` are not applied by the parser at all. It returns the BARE BASE plus
 * the step size and leaves the rotation to the caller — PatternEngine tracks
 * the offset, the processor rotates by -offset. So the probe printed the same
 * pattern on every trigger, three rows that read as reference vectors and could
 * never match a port that does advance.
 *
 * That was live for as long as the JS progressive port has existed, and the
 * port was verified against this probe. Comparing the two showed three
 * "divergences" in the newest, least-exercised code in the suite — every one
 * an artifact of the instrument reading the wrong layer. Driven properly, the
 * engine and the reference agree on every vector below, first try. The port
 * was right; the measurement was not.
 *
 * A probe is read by a person, so it can go quietly wrong again. This file is
 * the machine-checked version, and it is what should fail if either side
 * moves.
 *
 * PHASE (INTENT D6): trigger 1 is the BARE BASE, so vector 1 of every case is
 * the unrotated pattern and rotation starts at trigger 2. DIRECTION: positive
 * offset is clockwise, which is rotatePattern(base, -offset) — the sign is the
 * easiest thing here to get backwards, so `%-2` is included and is `%2` run
 * the other way round.
 *
 * Vectors are the JS reference's own output, from @enkerli/upi:
 *
 *   progressiveAt(parseProgressive(src), n, { parseBase: (s) => parseUPI(s) })
 */
#include <JuceHeader.h>
#include <cstdio>
#include <string>
#include <vector>
#include "UPIParser.h"
#include "PatternEngine.h"
#include "PatternUtils.h"

namespace {

struct Vec { const char* notation; int trigger; const char* expected; };

// Generated from @enkerli/upi — see the header comment.
const Vec kVectors[] = {
    { "E(3,8)%2", 1, "10010010" },
    { "E(3,8)%2", 2, "10100100" },
    { "E(3,8)%2", 3, "00101001" },
    { "E(3,8)%2", 4, "01001010" },
    { "E(3,8)%2", 5, "10010010" },   // 8/gcd(8,2) = 4, so it comes back round
    { "E(3,8)%2", 6, "10100100" },

    // `+N` is the legacy spelling of the same thing, and must stay identical.
    { "E(3,8)+3", 1, "10010010" },
    { "E(3,8)+3", 2, "01010010" },
    { "E(3,8)+3", 3, "01001010" },
    { "E(3,8)+3", 4, "01001001" },
    { "E(3,8)+3", 5, "00101001" },

    // Coprime step and length: 13 triggers before it repeats, so an off-by-one
    // in the offset shows up immediately rather than aliasing back.
    { "E(5,13)%5", 1, "1001010010100" },
    { "E(5,13)%5", 2, "1010010010100" },
    { "E(5,13)%5", 3, "1010010100100" },
    { "E(5,13)%5", 4, "0010010100101" },
    { "E(5,13)%5", 5, "0010100100101" },

    { "E(7,16)%3", 1, "1010100101010010" },
    { "E(7,16)%3", 2, "0101010100101010" },
    { "E(7,16)%3", 3, "0100101010100101" },
    { "E(7,16)%3", 4, "1010100101010100" },
    { "E(7,16)%3", 5, "1001010100101010" },

    // A polygon rotates like anything else — the base is whatever the notation
    // produced, expansion factor included.
    { "P(3,1,4)%2", 1, "010001000100" },
    { "P(3,1,4)%2", 2, "000100010001" },
    { "P(3,1,4)%2", 3, "010001000100" },
    { "P(3,1,4)%2", 4, "000100010001" },

    // Negative step: the same cycle as `%2`, walked backwards. Pins the sign.
    { "E(3,8)%-2", 1, "10010010" },
    { "E(3,8)%-2", 2, "01001010" },
    { "E(3,8)%-2", 3, "00101001" },
    { "E(3,8)%-2", 4, "10100100" },

    { "E(5,8)%3", 1, "10110110" },
    { "E(5,8)%3", 2, "11010110" },
    { "E(5,8)%3", 3, "11011010" },
    { "E(5,8)%3", 4, "01011011" },
};

std::string bits (const std::vector<bool>& p)
{
    std::string s;
    for (bool b : p) s += (b ? '1' : '0');
    return s;
}

} // namespace

int main()
{
    std::printf ("=== Progressive offset conformance (%%N / +N vs @enkerli/upi) ===\n\n");

    int checks = 0, failures = 0;
    juce::String currentNotation;
    PatternEngine engine;
    UPIParser::ParseResult base;

    for (const auto& v : kVectors)
    {
        // Each notation is one run: parse once, then step the engine. Vectors
        // for a notation must be listed in trigger order.
        if (currentNotation != v.notation)
        {
            currentNotation = v.notation;
            ProgressiveTransformState progressive;
            base = UPIParser::parse (juce::String (v.notation), progressive);

            if (! base.isValid())
            {
                std::printf ("  FAIL %-12s parse error: %s\n", v.notation, base.errorMessage.toRawUTF8());
                ++failures; ++checks;
                continue;
            }
            if (! base.hasProgressiveOffset)
            {
                std::printf ("  FAIL %-12s parsed, but not as a progressive offset —\n"
                             "       the notation stopped being recognised as one\n", v.notation);
                ++failures; ++checks;
                continue;
            }
            engine.setProgressiveOffset (true, base.initialOffset, base.progressiveOffset);
        }

        // Rotate from the freshly parsed base by the engine's derived offset,
        // as the poly path does. Negative = clockwise.
        const auto got = bits (PatternUtils::rotatePattern (base.pattern, -engine.getCurrentOffset()));
        ++checks;
        if (got != v.expected)
        {
            std::printf ("  FAIL %-12s trigger %d\n         expected %s\n         got      %s\n",
                         v.notation, v.trigger, v.expected, got.c_str());
            ++failures;
        }

        engine.triggerProgressiveOffset();   // phase: step AFTER, so trigger 1 is the base
    }

    std::printf ("\n%d checks, %d failure%s\n", checks, failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
