#!/usr/bin/env node
/**
 * gen-conformance-header — emit RhythmConformanceVectors.h from the vendored
 * cross-language vectors (WebApp/tests/rhythm-vectors.json), so the C++
 * conformance test embeds the data with no JSON parser dependency.
 *
 *   node Source/Tests/gen-conformance-header.mjs
 *
 * Commit the regenerated header. Refresh the JSON first (see WebApp/tests/README.md).
 */
import { readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const SRC = join(HERE, "..", "..", "WebApp", "tests", "rhythm-vectors.json");
const OUT = join(HERE, "RhythmConformanceVectors.h");

const doc = JSON.parse(readFileSync(SRC, "utf8"));
const cases = [...doc.codecs, ...doc.codecBatch];

const rows = cases
  .map((c) => `    { "${c.pattern}", ${c.steps}, ${c.decimal}u, "${c.hex}", "${c.octal}" }`)
  .join(",\n");

const header = `// AUTO-GENERATED from WebApp/tests/rhythm-vectors.json — do not edit by hand.
// Regenerate: node Source/Tests/gen-conformance-header.mjs
//
// Cross-language rhythm codec vectors (suite-wide, leftmost = LSB: first step
// = bit 0, so step k has value 2^k). Sourced from @enkerli/theory; the
// webapp PatternConverter is checked against the same JSON. See CONVENTIONS.md.
#pragma once
#include <cstdint>

struct RhythmCodecVector {
    const char* pattern;   // binary string, first step leftmost (= bit 0)
    int         steps;     // pattern.length
    uint64_t    decimal;   // value: sum of 2^k over set steps k
    const char* hex;       // bare numeral, uppercase, no 0x
    const char* octal;     // bare numeral, no 0o
};

// ${cases.length} cases (${doc.codecs.length} named + ${doc.codecBatch.length} batch).
static const RhythmCodecVector kRhythmCodecVectors[] = {
${rows}
};
static const int kRhythmCodecVectorCount =
    (int) (sizeof(kRhythmCodecVectors) / sizeof(kRhythmCodecVectors[0]));
`;

writeFileSync(OUT, header);
console.log(`✓ RhythmConformanceVectors.h — ${cases.length} vectors`);
