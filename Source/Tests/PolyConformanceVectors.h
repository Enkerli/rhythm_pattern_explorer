// AUTO-GENERATED from WebApp/tests/poly-vectors.json — do not edit by hand.
// Regenerate: node Source/Tests/gen-poly-conformance-header.mjs
//
// Cross-language vectors for Serpe's poly-lane notation (music-suite
// docs/SERPE_POLY.md). Sourced from packages/upi/src/poly.js via
// packages/upi/vectors/poly.json. See PolyParser.h/.cpp and CONVENTIONS.md.
#pragma once
#include <vector>

struct PolyOffsetVec {
    bool isMs = false;
    bool isFrac = false;
    int  ms = 0;
    int  num = 0;
    int  den = 0;
};

struct PolyLaneVec {
    const char*   label;
    const char*   steps;   // bit-string, leftmost = LSB
    PolyOffsetVec offset;
};

struct PolyCaseVec {
    const char*  input;
    bool         ok;
    int          lcm;
    std::vector<PolyLaneVec> lanes;
};

// 11 cases.
static const std::vector<PolyCaseVec> kPolyConformanceVectors = {
    { "E(4,16) / E(3,8) / {10}E(2,3)", true, 48, {
        { "lane1", "1000100010001000", PolyOffsetVec{} },
        { "lane2", "10010010", PolyOffsetVec{} },
        { "lane3", "110", PolyOffsetVec{} }
    } },
    { "kick=E(4,16) / snare=E(2,4)@+12ms / hat={10}E(8,16)@-1/64", true, 16, {
        { "kick", "1000100010001000", PolyOffsetVec{} },
        { "snare", "1010", PolyOffsetVec{ true, false, 12, 0, 0 } },
        { "hat", "1010101010101010", PolyOffsetVec{ false, true, 0, -1, 64 } }
    } },
    { "E(3,8)@-6", true, 8, {
        { "lane1", "10010010", PolyOffsetVec{ true, false, -6, 0, 0 } }
    } },
    { "E(3,8)@-1/64", true, 8, {
        { "lane1", "10010010", PolyOffsetVec{ false, true, 0, -1, 64 } }
    } },
    { "{100}E(3,8);12", true, 12, {
        { "lane1", "100001000100", PolyOffsetVec{} }
    } },
    { "E(2,3) / E(4,16)", true, 48, {
        { "lane1", "110", PolyOffsetVec{} },
        { "lane2", "1000100010001000", PolyOffsetVec{} }
    } },
    { "kick=E(4,16)@+1/32 / snare=E(2,4)", true, 16, {
        { "kick", "1000100010001000", PolyOffsetVec{ false, true, 0, 1, 32 } },
        { "snare", "1010", PolyOffsetVec{} }
    } },
    { "E(3,8)@+51ms", false, 0, {} },
    { "E(3,8)@+1/4", false, 0, {} },
    { "kick=E(4,16) / snare=nonsense(((", false, 0, {} },
    { "P(3,1)+P(5,0)", true, 15, {
        { "lane1", "110100100101100", PolyOffsetVec{} }
    } }
};
