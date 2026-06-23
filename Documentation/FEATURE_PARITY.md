# Serpe feature-parity ledger

The Claude Design redesign is a **visual skin**, not a spec. This ledger is the
recovery source of truth: every feature of the real engine and the original web
app, with its status in the new React WebView UI.

**Authorities** (not the design doc):
- **Engine (plugin):** `Source/Core/UPIParser.{h,cpp}`, `PatternEngine`,
  `QuantizationEngine`, `AccentSequence`; `Source/Managers/{Scene,Progressive,Preset}Manager`.
- **Original web app:** `WebApp/app/*.js` (~20k LOC) — the full JS implementation.
- **New UI:** `Source/WebUI/` (React) + `engine/{upi,rhythm,analysis,render}.js`.

**Status:** ✓ present & correct · ⚠ present but broken/partial · ✗ lost · ➕ new (added since redesign)

**The structural cause** (drives most ✗/⚠): the new `upi.js` is a *subset*
re-derivation. It (a) **rejects** notation the C++ engine accepts, so that input
never reaches the engine in the plugin; (b) **diverges** from what the engine
actually plays (accent phase). Two corrections close most of this:
1. **Plugin = engine-authoritative.** Send UPI straight to C++ (don't gate on
   `upi.js`); render `getCurrentPatternDisplay()` / real accents (`shouldStepBeAccented`) /
   `getCurrentStep()` / `getUIAccentOffset()`. The UI renders engine state.
2. **Webapp = full port.** Port `WebApp/app`'s engine (not the design subset).

---

## Notation (the UPI input language)

| Notation | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| `E(k,n)` / `E(k,n,rot)` Euclidean | ✓ | ✓ | ✓ | canonical port |
| `P(k,off[,n])` polygon | ✓ | ✓ | ✓ | |
| `R(k,n)` random | ✓ | ✓ | ✓ | |
| `B/W/D(k,n)` Barlow/Wolrab/Dilcue | ✓ | ✓ | ✓ | canonical port |
| binary / `b…` | ✓ | ✓ | ✓ | |
| hex `0x…[:N]` | ✓ | ✓ | ✓ | now emits `:N` (round-trips) |
| octal `0o…/o…[:N]` | ✓ | ✓ | ✓ | |
| decimal `d…[:N]` | ✓ | ✓ | ✓ | |
| onset array `[0,3,6]:N` | ✓ | ✓ | ✓ | |
| **Morse** (`.-`, `M:`, SOS/CQ, letter words) | ✓ `parseMorse*` | ✓ | ✓ (webapp) | ported to `upi.js` (`L:` custom durations still TODO) |
| accents `{…}` (onset-cyclic) | ✓ | ✓ | ⚠ | display phase off-by-one vs audio (cycle 2+); see Accents |
| **progressive offset** `pat+N` / `pat%N` | ✓ | ✓ | ✗ | UI "Progressive" is local JS, not the notation |
| **progressive transform** `pat>N` (B/W/E/D) | ✓ `applyProgressiveTransformation` | ✓ | ✗ | `upi.js` doesn't parse `>` → gated out |
| **pattern combination** `pat+pat` / `pat-pat` (LCM) | ✓ (fixed `-`) | ✓ | ✓ (webapp) | engine `-` fixed; ported to `upi.js` (union/diff, polygon-LCM) |
| shorthand names (`tresillo`, `tri/pent/hex…`) | ✓ (fixed) | ✓ | ✓ (webapp) | engine fixed (Morse order); ported to `upi.js` |

## Generators / transforms

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Euclid/Polygon/Random/Barlow/Wolrab/Dilcue gen | ✓ | ✓ | ✓ | |
| Dilute / Concentrate (Barlow + modes) | ✓ | ✓ | ✓ | modes restored (Barlow/Wolrab/Euclid/Dilcue) |
| **Funkifier** (FunkyEuclidean / funkiness) | — | ✓ | ✓ (webapp+plugin) | ported → `rhythm.js` `funkyEuclidean`; "Funk" generator type + Funkiness slider |
| **Syncopation** analysis (6 measures + overall) | — | ✓ `SyncopationAnalyzer` | ✓ (webapp+plugin) | ported verbatim → `engine/syncopation.js`, shown in Analysis |
| Quantization `;N` / `;-N` | ✓ `QuantizationEngine` | ✓ `quantize` | ✓ (webapp) | ported (Lascabettes angular) → `upi.js` `quantizeSteps` |
| Rotate / Invert / Complement | ✓ | ✓ | ✓ | |

## Progressive / scenes

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Progressive offset | ✓ `ProgressiveManager` | ✓ | ⚠ | local JS rotate, not engine; loses accents historically |
| Progressive lengthening | ✓ | ✓ `progressiveLengthening` | ⚠ | local JS; accents now kept, but not engine-driven |
| Progressive transform `>` | ✓ | ✓ | ✗ | gated by `upi.js` |
| Scenes (`|` notation, advance) | ✓ `SceneManager`, `advanceScene` | ✓ `advanceScene` | ⚠ | UI scenes are local; **accents not kept**; no MIDI-note advance |

## Analysis

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Perfect balance / CoG | — | ✓ | ✓ | |
| Euclidean evenness | — | ✓ | ✓ | |
| Barlow indispensability table | ✓ | ✓ | ✓ | canonical (works on primes) |
| **Syncopation analysis** | — | ✓ `analyzeSyncopation` | ✓ | `engine/syncopation.js` (6 measures), in Analysis panel |

## Library / data

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Pattern **database** (search/filter/stats/balance categories) | — | ✓ `pattern-database.js` | ⚠ | new minimal presets/library/history ≠ original DB |
| Presets | — | ✓ | ➕ | added (curated list) |
| Saved library | — | ✓ | ➕ | added (localStorage) |
| History | — | ✓ | ➕ | added (localStorage) |

## Playback / output

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| MIDI note / channel | ✓ `midiNote` | ✓ | ✓ / ⚠ | channel is UI-local (no APVTS param yet) |
| Accents: velocity / pitch offset | ✓ | ✓ | ✓ | params wired |
| Accents: onset-indexed + precession | ✓ `uiAccentOffset`, `globalOnsetCounter` | ✓ | ⚠ | **don't sync with audio**; cycle-2 off-by-one |
| **Tick** voice / pattern update | ✓ `tickParam` | ✓ | ✗ | "tick doesn't update a pattern" |
| Host transport sync + step duration | ✓ | n/a | ✓ | Step length → subdivision (Steps mode) |
| Web Audio (standalone) | n/a | ✓ | ✓ | webapp only |
| WebRTC-OSC (standalone) | n/a | ✓ | ✗ | flagged for removal (confirm) |
| Editable steps (tap to toggle) | ✓ `togglePatternStep` | ✓ | ✓ | lane + circle, in-place |
| Accent-edit gesture | ✓ `toggleAccentAtStep` | ✓ | ✓ | "accent" mode |

---

## Recovery order (OSC dropped; webapp engine to be ported)

1. ✅ **Plugin engine-authoritative display** (`37384cb` — verify in-host).
   Plugin sends raw UPI to C++ (no JS gate) and renders the engine's real pattern
   + per-step accents (`engineState`, from `shouldStepBeAccented` = the MIDI
   path). Should fix accents-don't-sync + cycle-2 off-by-one and make Morse / `>`
   / progressive / combinations / shorthand work at the input *in the plugin*.
   Webapp still uses the JS subset until move 3. Builds + auval clean; webapp no
   regression. **Test in Bitwig VST3 / AUM AUv3.**
2. **Scenes via the engine** (accents kept; MIDI-note advance) and **Tick**.
3. **Port the webapp engine** into `engine/` for the standalone.
   - ✅ Notation parity in `upi.js`: shorthand names (`24b1ef5`), combinations
     `+`/`-` incl. polygon-LCM (`24b1ef5`), Morse (`1322ae8`). All verified in
     node against the C++ engine.
   - ✅ Quantization `;N`/`;-N` (`7421b3e`, Lascabettes angular, cross-checked
     vs C++) and **syncopation** analysis (verbatim port → `engine/syncopation.js`,
     shown in the Analysis panel).
   - ✅ **Funkifier** (`rhythm.js` `funkyEuclidean`): "Funk" generator type +
     Funkiness slider — Euclidean base, random hit-nudges, backbeats, shuffle.
   - ⬜ Still to port: progressive notation `>` / `+N` / `%N` (stateful; the UI
     already has Progressive buttons, so this is wiring the notation to them).
4. **Reconcile the library** with the original database (stats/balance filters).
5. Copy fixes (e.g. `E(5,8)` is **cinquillo**, not "tresillo+").

### Cross-cutting fixes
- ✅ **Bit order reverted to leftmost = LSB** (`ee70bef`; theory `ea4929a` —
  verify in-host). `0x1:4` = `1000`, tresillo = `0x94`. Undoes the unreleased
  MSB-first switch across plugin + both webapps + `@enkerli/theory`; PCS stays
  MSB-first. 134 codec vectors pass (theory, C++, webapp); auval clean.

### Still open from the 2026-06-22 in-host report (Move 1 follow-ups)
- ✅ **Dilute/Concentrate Barlow/Wolrab on primes** — `engine/rhythm.js`
  `positionIndispensability` now uses the SAME heuristic as the C++ engine
  (`PatternUtils::calculateBarlowIndispensability`), breaking ties on prime
  meters. Barlow + Wolrab dilute/concentrate on 17 now reproduce the engine's
  `B>17`/`W>17` sequences exactly (verified). (theory keeps the academic
  stratified method — divergence is intentional, Serpe matches its own engine.)
- ✅ **B(k,n) / W(k,n) / D(k,n) generate** — the greedy Morse matcher
  (`validateMorsePattern`, `containsAnyOf("a-z")`) swallowed `b/w/d(…)` before
  the Barlow/Wolrab/Dilcue branches (E escaped because Euclidean is checked
  before Morse). Excluded parens from Morse. Verified: `B(3,8)`=10001001,
  `D(3,8)`=01001001, `B(5,17)` proper on the prime; Morse/`SOS`/`M:`/`.-`
  unaffected. NOTE: same greedy Morse also eats **shorthand names**
  (`tresillo` → "Morse: tresillo") — fix with the shorthand port (notation row).
- ✅ **Step-view balanced rows** — `render.js` wrapped at a hard 16-col cap
  (18→16+2, 19→16+3) while ≤16 stayed one row. Now splits into the fewest rows
  of ≤16, balanced: 16→one row, 18→9+9, 19→10+9, 32→16+16 (verified in browser).
  Also fixed stale example chips: `E(5,8)` mislabel `tresillo+`→`cinquillo`,
  and `0x92`→`0x94` (left over from the bit-order change).
- ✅ **macOS host icon** — `ICON_BIG` pointed at the full-bleed master
  (`serpe-1024-bleed.png`); JUCE doesn't round corners for the `.icns`, so it
  rendered as a hard square in desktop hosts. Repointed to the already-existing
  rounded master `serpe-1024.png` (matches PitchFold). Regenerated `.icns`
  verified rounded; installed VST3/AU refreshed.
- ✅ **iPadOS-26 AUv3 icon** (verify on device) — appex icons there must come
  from a compiled asset catalog with Any/Dark/Tinted appearances (loose
  CFBundleIcons are ignored). First cut added a *second* catalog → actool
  "None of the input catalogs contained … SerpeAppIcon" (build break). Corrected:
  CMake now **overwrites JUCE's generated `AppIcon.appiconset`** with our
  appearance version (`Assets/Serpe.xcassets/SerpeAppIcon.appiconset`) — one
  catalog, default `AppIcon` name. `actool`-verified on the host: compiles with
  the three appearances, no errors. Needs the maintainer's signed iPad build +
  AUM check. See ICON_iPadOS26.md.
- ✅ **Full-pattern duration sync** — the UI only had "Step length", which hard-
  set `patternLengthUnit=Steps`, so Beats/Bars (whole pattern = N beats/bars) was
  unreachable. Replaced with a "Pattern length" control: unit = Steps / Beats /
  Bars / Auto, with a conditional value (Steps → subdivision; Beats/Bars →
  length value; Auto → none). Bars = 1 → one bar. Wired to the existing C++
  `patternLengthUnit` / `patternLengthValue` params; snapshot-initialised.
  Verified in browser (all four modes show the right control).
- ✅ **Accent sync / cycle-2 off-by-one** (verify in-host by ear). Root cause:
  Move 1's `sendEngineState` displayed `shouldStepBeAccented` (a STATIC per-step
  array), but in normal mode the MIDI accents via `shouldOnsetBeAccented(onset
  count)` which PRECESSES across cycles — so cycle 1 matched, cycles 2+ drifted
  by one. Fixed: `sendEngineState` now mirrors `processStep`'s exact branch
  (per-step in suspension mode; onset-based at the live `getUIAccentOffset()` in
  normal mode), so the highlighted accents match what's heard every cycle.
  Static precession (`{10}E(5,8)`) confirmed ✅ in-host after `efb163d`.
  Follow-up for the **pattern-shift** case (`{100}B(0,17)>17`): the display used
  the boundary-latched `uiAccentOffset`, which a progressive change throws off
  (it resets `baseTickRhythm` mid-cycle). Now the display computes each onset's
  accent as `shouldOnsetBeAccented(getCycleStartOnsetCount() + onsetIndex)` —
  the exact global onset count the audio uses — so it can't latch at the wrong
  time or drift on a shift. `getCycleStartOnsetCount()` = completeCycles ×
  onsetsPerCycle from the live tick/base. Builds + auval clean; **verify the
  shift case in-host.**

Mark items here as they land; this file — not the design doc — is the scope.
