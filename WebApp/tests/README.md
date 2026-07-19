# Conformance tests

## `rhythm-conformance.mjs` — numeric notation cannot drift from the suite

```bash
node WebApp/tests/rhythm-conformance.mjs
```

Checks the webapp's `PatternConverter` against `rhythm-vectors.json` — the
suite-wide cross-language contract for numeric pattern notation
(binary / decimal / hex / octal / onset-array), **leftmost = LSB**: the first
step is bit 0, so step k has value 2^k — patterns read strictly left to right
(`10010010` = tresillo = `0x94` = `73`). See `CONVENTIONS.md` in the
`music-suite` repo.

The Serpe plugin's C++ (`UPIParser` / `PatternUtils`) is checked against the
**same** JSON, so the webapp and plugin can't disagree on bit order — the kind
of discrepancy that has bitten this project before.

### `rhythm-vectors.json` is vendored — don't hand-edit

It is the reference output of `@enkerli/theory` (`music-suite`,
`packages/theory/vectors/rhythm.json`). To refresh after the reference changes:

```bash
# in music-suite
npm run build -w @enkerli/theory
node packages/theory/vectors/gen-rhythm-codecs.mjs
# then copy the canonical file here
cp packages/theory/vectors/rhythm.json \
   ../rhythm_pattern_explorer/WebApp/tests/rhythm-vectors.json
```

## `poly-vectors.json` — poly-lane notation cannot drift from the webapp

Docs/SERPE_POLY.md (`music-suite`) §8 milestone 1: the plugin's `PolyParser`
(`Source/Core/PolyParser.h/.cpp`) is checked against `poly-vectors.json` — the
lane-splitting/label/offset/lcm surface of the poly notation (`kick=E(4,16) /
snare=E(2,4)@+12ms / hat={10}E(8,16)@-1/64`). Run via `ctest -R
poly_conformance`, or directly:

```bash
build-linux/serpe_poly_conformance_artefacts/serpe_poly_conformance   # or build-macos/…
```

`poly-vectors.json` is vendored — don't hand-edit. It is the reference output
of `@enkerli/upi`'s `parsePolyUPI` (`music-suite`,
`packages/upi/vectors/poly.json`). To refresh after the reference changes:

```bash
# in music-suite
node packages/upi/vectors/gen-poly-vectors.mjs
cp packages/upi/vectors/poly.json \
   ../rhythm_pattern_explorer/WebApp/tests/poly-vectors.json
# then in this repo, regenerate the embedded C++ header
node Source/Tests/gen-poly-conformance-header.mjs
```

Scope note: these vectors cover lane structure only (splitting, labels,
offsets, lcm) — each lane's underlying pattern (Euclidean/Polygon/accents/
etc.) is delegated to the existing, separately-conformant `UPIParser::parse`,
so accent-cycling detail is deliberately not re-asserted here.
