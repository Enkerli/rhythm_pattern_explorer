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
