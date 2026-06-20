#!/usr/bin/env node
/**
 * Rhythm codec conformance — webapp side.
 *
 * Loads the webapp's PatternConverter (app/pattern-processing.js) and checks it
 * reproduces every codec vector in rhythm-vectors.json, in both directions, for
 * binary / decimal / hex / octal / onset-array. The vectors are the suite-wide
 * cross-language contract (strict MSB-first: first step = leftmost bit = MSB),
 * sourced from @enkerli/theory; the plugin's C++ UPIParser is checked against
 * the same JSON. If this fails, the webapp has drifted from the convention.
 *
 *   node WebApp/tests/rhythm-conformance.mjs
 *
 * Exit 0 = all vectors pass; exit 1 = at least one mismatch (details printed).
 *
 * pattern-processing.js is a browser script (no module exports); it is loaded in
 * a vm sandbox with a stub `window`, and PatternConverter is read back out. No
 * change to the app source is needed.
 */
import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import vm from "node:vm";

const HERE = dirname(fileURLToPath(import.meta.url));
const vectors = JSON.parse(readFileSync(join(HERE, "rhythm-vectors.json"), "utf8"));

// Load PatternConverter out of the browser script without touching app source.
const src = readFileSync(join(HERE, "..", "app", "pattern-processing.js"), "utf8");
const sandbox = { window: {}, console };
vm.createContext(sandbox);
new vm.Script(src + "\n;this.PatternConverter = PatternConverter;").runInContext(sandbox);
const PC = sandbox.PatternConverter;
if (!PC) throw new Error("could not load PatternConverter from pattern-processing.js");

// The webapp prefixes hex with "0x" and octal with "o"; the vectors are bare
// numerals. Strip prefixes so we compare the numeral, not the notation sugar.
const bareHex = (s) => s.replace(/^0x/i, "").toUpperCase();
const bareOctal = (s) => s.replace(/^0o/i, "").replace(/^o/i, "");

let pass = 0;
const failures = [];
const eq = (a, b) => JSON.stringify(a) === JSON.stringify(b);

const check = (c) => {
  const bits = [...c.pattern].map((ch) => ch === "1");
  const name = c.name ?? c.pattern;
  const fail = (what, got, want) =>
    failures.push(`${name}: ${what} → got ${JSON.stringify(got)}, want ${JSON.stringify(want)}`);

  // encode
  if (PC.toBinary(bits, c.steps) !== c.pattern) fail("toBinary", PC.toBinary(bits, c.steps), c.pattern);
  if (PC.toDecimal(c.pattern) !== c.decimal) fail("toDecimal", PC.toDecimal(c.pattern), c.decimal);
  if (bareHex(PC.toHexFromBinary(c.pattern)) !== c.hex) fail("toHex", bareHex(PC.toHexFromBinary(c.pattern)), c.hex);
  if (bareOctal(PC.toOctalFromBinary(c.pattern)) !== c.octal) fail("toOctal", bareOctal(PC.toOctalFromBinary(c.pattern)), c.octal);
  if (PC.toOnsetArray(bits, c.steps) !== `[${c.onsets.join(",")}]`) fail("toOnsetArray", PC.toOnsetArray(bits, c.steps), `[${c.onsets.join(",")}]`);

  // decode (step count carried explicitly, matching the :N notation)
  if (!eq(PC.fromBinary(c.pattern).steps, bits)) fail("fromBinary", PC.fromBinary(c.pattern).steps, bits);
  if (!eq(PC.fromDecimalWithSteps(c.decimal, c.steps).steps, bits)) fail("fromDecimal", PC.fromDecimalWithSteps(c.decimal, c.steps).steps, bits);
  if (!eq(PC.fromHex(`${c.hex}:${c.steps}`).steps, bits)) fail("fromHex", PC.fromHex(`${c.hex}:${c.steps}`).steps, bits);
  if (!eq(PC.fromOctal(`${c.octal}:${c.steps}`).steps, bits)) fail("fromOctal", PC.fromOctal(`${c.octal}:${c.steps}`).steps, bits);

  pass++;
};

for (const c of vectors.codecs) check(c);
for (const c of vectors.codecBatch) check(c);

if (failures.length) {
  console.error(`✗ rhythm codec conformance FAILED — ${failures.length} mismatch(es):`);
  for (const f of failures.slice(0, 30)) console.error("  " + f);
  if (failures.length > 30) console.error(`  …and ${failures.length - 30} more`);
  process.exit(1);
}
console.log(`✓ rhythm codec conformance — webapp PatternConverter matches all ${pass} vectors (${vectors.codecs.length} named + ${vectors.codecBatch.length} batch).`);
