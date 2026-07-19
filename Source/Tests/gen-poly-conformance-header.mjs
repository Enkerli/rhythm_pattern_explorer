#!/usr/bin/env node
/**
 * gen-poly-conformance-header — emit PolyConformanceVectors.h from the
 * vendored cross-language vectors (WebApp/tests/poly-vectors.json), so the
 * C++ conformance test embeds the data with no JSON parser dependency.
 *
 *   node Source/Tests/gen-poly-conformance-header.mjs
 *
 * Commit the regenerated header. Refresh the JSON first (see WebApp/tests/README.md).
 */
import { readFileSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const SRC = join(HERE, "..", "..", "WebApp", "tests", "poly-vectors.json");
const OUT = join(HERE, "PolyConformanceVectors.h");

const doc = JSON.parse(readFileSync(SRC, "utf8"));

const cEscape = (s) => s.replace(/\\/g, "\\\\").replace(/"/g, '\\"');

const offsetLiteral = (o) => {
  if (!o) return "PolyOffsetVec{}";
  if (o.kind === "ms") return `PolyOffsetVec{ true, false, ${o.ms}, 0, 0 }`;
  return `PolyOffsetVec{ false, true, 0, ${o.num}, ${o.den} }`;
};

const laneLiteral = (l) =>
  `        { "${cEscape(l.label)}", "${l.steps}", ${offsetLiteral(l.offset)} }`;

const caseLiteral = (c) => {
  if (!c.ok) {
    return `    { "${cEscape(c.input)}", false, 0, {} }`;
  }
  const lanes = c.lanes.map(laneLiteral).join(",\n");
  return `    { "${cEscape(c.input)}", true, ${c.lcm}, {\n${lanes}\n    } }`;
};

const rows = doc.cases.map(caseLiteral).join(",\n");

const header = `// AUTO-GENERATED from WebApp/tests/poly-vectors.json — do not edit by hand.
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

// ${doc.cases.length} cases.
static const std::vector<PolyCaseVec> kPolyConformanceVectors = {
${rows}
};
`;

writeFileSync(OUT, header);
console.log(`wrote ${OUT} (${doc.cases.length} cases)`);
