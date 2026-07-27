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
#include <cstdio>
#include <string>
#include <vector>

static void probe (const char* input)
{
    auto r = UPIParser::parse (juce::String (input));
    std::string bits;
    for (bool b : r.pattern) bits += (b ? '1' : '0');

    std::printf ("%-26s ", input);
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

int main()
{
    std::printf ("=== UPIParser probe — what the C++ engine actually returns ===\n\n");
    const std::vector<const char*> inputs {
        // Baselines that are known to work
        "E(3,8)", "M:SOS", "...-",
        // The aksak notation
        "A(2,2,2,3)", "A(2,2,3,2)", "A(2,2,3)",
        // Custom durations, both spellings, and the failing trailing-dot case
        "L:2,3 ...-", "D:2,3 ...-", "L:2,3 ...-.", "D:2,3 ...-.",
        "L:1,5 .-", "D:3,1 .-",
        // Feel suffixes, alone and attached
        "E(3,8) PD(50%)", "E(3,8) PD(90%)", "A(2,2,2,3) PD(60%)",
        "E(3,8) LS(0.5)", "E(3,8) LS(1.4..1.8, 70%)",
        "E(3,8) LS(1.4..1.8) PD(30%)",
        // Things that must NOT be mistaken for the above. The generators B/W/D
        // share letters with Morse; `d…` is also the decimal prefix (that
        // collision is what ate "D:2,3 ...-"); and '+'/'-' between whole
        // patterns must still combine even though '-' is a Morse dash.
        "PD(50%)", "B(3,8)", "W(3,8)", "D(3,8)",
        "d73", "0x94:8", "E(3,8)+P(3,0)", "E(5,8)-E(3,8)", "1010-0011",
        "E(3,8)+P(3,0) PD(40%)",
        // Polygons in a combination: shapes projected onto the target length,
        // never tiled. E(3,8)+P(4,0) is the README's own example.
        "P(3,0)", "P(4,0)", "E(3,8)+P(4,0)", "P(3,0)+P(5,0)", "P(3,0,8)+E(3,8)",
        "P(3,1)+P(5,0)+P(2,5)", "E(3,8)+E(2,4)",
        // Third polygon argument is an EXPANSION FACTOR (k*x steps), per the
        // original webapp's "Triangle x4" help text. `;N` is the re-grid.
        "P(3,1,4)", "P(3,0,8)", "P(3,0);8", "P(3,0)+P(5,1)-P(2,0)",
    };
    for (auto* in : inputs) probe (in);
    std::printf ("\n(compare against the webapp / `msuite upi` for the same strings)\n");
    return 0;
}
