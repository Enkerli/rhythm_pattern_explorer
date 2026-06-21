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
| **Morse** (`.-`, `L:` durations, accents) | ✓ `parseMorse*` | ✓ | ✗ | `upi.js` has none → rejected, never sent to C++ |
| accents `{…}` (onset-cyclic) | ✓ | ✓ | ⚠ | display phase off-by-one vs audio (cycle 2+); see Accents |
| **progressive offset** `pat+N` / `pat%N` | ✓ | ✓ | ✗ | UI "Progressive" is local JS, not the notation |
| **progressive transform** `pat>N` (B/W/E/D) | ✓ `applyProgressiveTransformation` | ✓ | ✗ | `upi.js` doesn't parse `>` → gated out |
| **pattern combination** `pat+pat` / `pat-pat` (LCM) | ✓ | ✓ `combineMultiplePatterns` | ✗ | not ported |
| shorthand names (`tresillo`, `tri/pent/hex…`) | ✓ | ✓ | ✗ | not ported |

## Generators / transforms

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Euclid/Polygon/Random/Barlow/Wolrab/Dilcue gen | ✓ | ✓ | ✓ | |
| Dilute / Concentrate (Barlow + modes) | ✓ | ✓ | ✓ | modes restored (Barlow/Wolrab/Euclid/Dilcue) |
| **Funkifier** (FunkyEuclidean / funkiness) | — | ✓ `FunkinessSlider`, `FunkyPattern` | ✗ | port from webapp |
| **Syncopation** (add/analyze/apply/generate/mutate) | — | ✓ `*Syncopation*` (`pattern-analysis.js`) | ✗ | port from webapp |
| Quantization | ✓ `QuantizationEngine` | ✓ `quantize` | ✗ | not exposed |
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
| **Syncopation analysis** | — | ✓ `analyzeSyncopation` | ✗ | port |

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

## Recovery order (proposed — confirm before implementing)

1. **Plugin engine-authoritative display** — render the engine's real pattern +
   accents + phase; pass UPI straight through. → fixes accents-don't-sync, the
   cycle-2 off-by-one, and unblocks Morse / `>` / progressive / combinations /
   shorthand at the input in one move.
2. **Scenes via the engine** (accents kept; MIDI-note advance) and **Tick**.
3. **Port the webapp engine** into `engine/` for the standalone: Morse, `>`,
   combinations, shorthand, syncopation, funkifier, quantization.
4. **Reconcile the library** with the original database (stats/balance filters).
5. Copy fixes (e.g. `E(5,8)` is **cinquillo**, not "tresillo+").

Mark items here as they land; this file — not the design doc — is the scope.
