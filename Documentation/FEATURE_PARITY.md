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

> **Columns, as of 2026-07-27.** "Webapp" and "New UI" were separate
> implementations when this ledger was written. They are now the SAME code:
> `@enkerli/upi` in the music-suite monorepo, which the standalone webapp, the
> plugin's WebView UI, and `msuite upi` all import. Read the two columns as one
> — "the shared JS engine" — except where a row says otherwise.
>
> **Differential test, 2026-07-27 (this is measured, not read).** Every case in
> the C++ parser probe (`serpe_parser_probe`, built by the normal cmake build)
> was replayed through the JS engine via `node packages/cli/dist/cli.js upi`:
>
> ```
> serpe_parser_probe | replay each input through `msuite upi` | diff the bits
> ```
>
> **62 of 62 real patterns produced identical bit strings**, plus 2 inputs both
> engines correctly reject. Covered: Euclidean · polygons incl. the `P(k,off,x)`
> expansion factor · Barlow/Wolrab/Dilcue · Morse (`M:`, bare `.-`) · `A(...)`
> additive/aksak · `L:`/`D:` long-short · `PD()` and `LS()` feel suffixes ·
> multi-operand combination with LCM projection · `;N`/`;-N` quantization ·
> every codec (hex/octal/decimal/binary/onset-array, with and without `:N`) ·
> and the leftmost-LSB convention (`0x1` → `1000`, `d5` → `101`).
> **Progressive closed 2026-07-27.** It was the last ✗: stateful notation the
> pure parser could not represent. `@enkerli/upi/progressive.js` makes the
> state *derivable* (a function of the trigger index) instead of stored, so
> `progressiveAt(desc, n)` stays pure; the probe grew a progressive section
> that prints N successive triggers, and the transform sequences are pinned
> bit-for-bit. `msuite upi "E(1,8)>8" --triggers 9` now prints the progression
> rather than "Unrecognised pattern". Two honest residuals are in the rows
> below: offset *direction* and lengthening randomness.

**The structural cause** (drives most ✗/⚠): the new `upi.js` is a *subset*
re-derivation. It (a) **rejects** notation the C++ engine accepts, so that input
never reaches the engine in the plugin; (b) **diverges** from what the engine
actually plays (accent phase). Two corrections close most of this:
1. **Plugin = engine-authoritative.** ✅ **DONE** (raw `setUPI` send + engineState
   render landed with the monorepo consolidation 2026-06-28; the missing
   *re-trigger* — Enter in the UPI field and, with engine notation, the Advance
   button re-send the same string, which is the engine's advance semantics
   shared with Tick and MIDI-in — landed 2026-07-01). The UI renders engine
   state; `upi.js` remains a soft hint only.
2. **Webapp = full port.** Port `WebApp/app`'s engine (not the design subset).
   Still open where marked below (webapp column).

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
| accents `{…}` (onset-cyclic) | ✓ | ✓ | ✓ | display now mirrors the audio's accent branch exactly (see Accents row + tail note) |
| **progressive offset** `pat+N` / `pat%N` / `pat*N` | ✓ | ✓ | ✓ | **Closed 2026-07-27; direction and phase settled 2026-07-28, now bit-identical.** The ⚠ here said the rotation direction could not be differential-tested because offset state lives in PatternEngine, which the parser probe cannot reach. A **poly lane** reaches it, so `serpe_poly_precedence` became the missing harness — and both halves turned out to be wrong. **Sign:** JS `rotate(p, +k)` equals C++ `rotatePattern(p, -k)`; the two helpers were written with opposite conventions. **Phase:** the engine shows offset N on the *first* trigger, not the un-rotated base, so the JS ran a trigger behind the plugin. JS moved to the engine (Alex: engine-authoritative). `E(3,8)%2` is now pinned as a vector: `10100100 00101001 01001010 10010010 10100100`. Lengthening (`*N`) took the same phase fix — trigger 1 is base + step, matching a scene entering `E(3,8)*3` at 11 steps — but stays structural, never bit-equal, because both sides are random by design |
| **progressive transform** `pat>N` (B/W/E/D) | ✓ `applyProgressiveTransformation` | ✓ | ✓ | **Closed 2026-07-27, bit-identical.** `progressiveAt(desc, n)` reproduces the engine's own sequences verbatim — `E(1,8)>8`, `B(1,17)>17`, `E(8,8)>1` are pinned as vectors taken from `serpe_parser_probe` (which now prints N successive triggers). Includes the ±1-onset walk, the dilution direction, the transformer letter before `>` defaulting to Barlow, and the **loop back to base** on reaching the target |
| **pattern combination** `pat+pat` / `pat-pat` (LCM) | ✓ (fixed `-`) | ✓ | ✓ (webapp) | engine `-` fixed; ported to `upi.js` (union/diff, polygon-LCM) |
| shorthand names (`tresillo`, `tri/pent/hex…`) | ✓ (fixed) | ✓ | ✓ (webapp) | engine fixed (Morse order); ported to `upi.js` |
| **`A(2,2,3,2)` additive / aksak** | ✓ | ✓ | ✓ | beat GROUPS, one onset each — the Balkan 9/8 is `A(2,2,2,3)`. More general than `L:`/`D:`, which only say short-or-long. JS landed 2026-07-26, C++ 07-27; bit-identical (differential above) |
| **`L:s,l` / `D:s,l` custom durations** | ✓ | ✓ | ✓ | the two-value long/short form the ledger used to list as "TODO" on the Morse row; `D:` is accepted as an alias and reported as `L:` |
| **`PD(n%)` feel** (participatory discrepancy) | ✓ | ✓ | ✓ | a micro-timing depth carried alongside the pattern, not a change to the bits — the differential confirms identical bits with the feel attached |
| **`LS(lo..hi, n%)` long-short feel** | ✓ | ✓ | ✓ | as above; `LS(2)` and the range form both parse |
| `P(k,off,x)` expansion factor | ✓ | ✓ | ✓ | `x` multiplies the step count (`P(7,2,2)` = 14 steps), distinct from tiling — the 07-27 combination fixes made bare polygons *project* onto the shared cycle rather than repeat |

## Generators / transforms

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Euclid/Polygon/Random/Barlow/Wolrab/Dilcue gen | ✓ | ✓ | ✓ | |
| Dilute / Concentrate (Barlow + modes) | ✓ | ✓ | ✓ | modes restored (Barlow/Wolrab/Euclid/Dilcue) |
| **Funkifier** (FunkyEuclidean / funkiness) | — | ✓ | ✓ (webapp+plugin) | ported → `rhythm.js` `funkyEuclidean`; "Funk" generator type + Funkiness slider |
| **Syncopation** analysis (6 measures + overall) | — | ✓ `SyncopationAnalyzer` | ✓ (webapp+plugin) | ported verbatim → `engine/syncopation.js`, shown in Analysis |
| Quantization `;N` / `;-N` | ✓ `QuantizationEngine` | ✓ `quantize` | ✓ (webapp) | ported (Lascabettes angular) → `upi.js` `quantizeSteps` |
| Rotate / Invert / Complement / **Retrograde** | ✓ | ✓ | ✓ | retrograde (reverse) added |
| **Mutate** (balanced/groove/syncopate/straighten/swing/shuffle) | — | ✓ `RhythmMutator` | ✓ | ported → `engine/mutate.js`; style + amount + Mutate button |

## Progressive / scenes

| Feature | Engine | Webapp | New UI | Notes / fix |
|---|---|---|---|---|
| Progressive offset | ✓ `ProgressiveManager` | ✓ | ✓ plugin / ⚠ webapp | plugin: `%N`/`+N` notation engine-driven, Enter/Advance re-trigger (2026-07-01). Webapp + the slider-driven Progressive section stay local JS |
| Progressive lengthening | ✓ | ✓ `progressiveLengthening` | ✓ plugin / ⚠ webapp | plugin: `*N` notation engine-driven (bell-curve growth is the engine's). Slider path local (accents kept) |
| Progressive transform `>` | ✓ | ✓ | ✓ plugin / ✗ webapp | engine-driven via raw send + re-trigger |
| Scenes (`|` notation, advance) | ✓ `SceneManager`, `advanceScene` | ✓ `advanceScene` | ✓ plugin / ⚠ webapp | plugin: typed `a\|b\|c` uses the engine's SceneManager (per-scene accents preserved); Enter/Tick/**MIDI-note advance** all work (engine-side). UI scene pads remain a separate local pattern bank (➕). Webapp typed `\|` still unparsed |

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
| Accents: onset-indexed + precession | ✓ `uiAccentOffset`, `globalOnsetCounter` | ✓ | ✓ | fixed — display mirrors `processStep`'s exact branch; static precession confirmed in-host (see tail note); pattern-shift case uses `getCycleStartOnsetCount` |
| **Tick** voice / pattern update | ✓ `tickParam` | ✓ | ✓ plugin | tick edge advances scenes/progressive in `processBlock` (same path as Enter/MIDI-in); display follows via change-detected `sendEngineState` (editor timer). Verify once in-host by automating Tick |
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
2. ✅ **Scenes via the engine** (accents kept; MIDI-note advance) and **Tick**
   (2026-07-01): typed notation was already engine-side after move 1; the UI
   re-trigger (Enter / Advance-with-notation) was the missing piece. Scene
   *pads* stay a local pattern bank (➕ feature, not the `|` notation).
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

- ✅ **Engine-authoritative re-trigger (2026-07-01).** The raw-send path (06-28)
  meant typed scenes/progressive notation *reached* the engine, but the new UI
  only sent on text change — nothing could advance the same string. Now:
  **Enter** in the UPI field re-sends (engine semantics: scenes advance, `>N`
  steps, `%N` offsets — the same code path as Tick and MIDI-note-in), and the
  Progressive section's **Advance cycle** button re-sends instead of local-JS
  rotating whenever the field carries engine notation (`|`, `>`, or a numeric
  `%`/`+`/`*` tail); the slider-driven local model is unchanged (webapp parity).
  UI change only (music-suite `apps/serpe/main.jsx`); no C++ touched. Ladder:
  monorepo 847 tests, macOS build, **auval PASS**, **pluginval 8 s10 zero
  failures (VST3)**, standalone WKWebView smoke clean, webapp browser smoke
  clean (Enter = no-op re-parse, `0x94:8` = tresillo d73). **Pending device
  session:** by-ear scene/progressive advance in AUM (checklist §5) and a
  host-automated Tick pass.

---

## Engine archaeology — progressive offset, 2026-07-27 (read before "cleaning up")

> **RESOLVED 2026-07-28.** `triggerProgressiveOffset()` now has a caller: the
> poly lane path calls it once per trigger (Serpe `f1917a1`), so the engine's
> own progressive-offset machinery is live rather than configured-and-ignored.
> That also supplied the harness this section says was missing, and settled
> both the rotation sign and the phase — see the `pat%N` row above. The mono
> path still rotates via the processor's own `progressiveOffset` int rather
> than the engine's, so the duplication described below is real but now only
> half of it is inert. Kept because the reasoning still applies.

Found while porting progressive notation to JS. **Nothing was changed**; this
is a record so the next person doesn't mistake a half-finished migration for
dead code, or trust a path that isn't running.

**`PatternEngine::triggerProgressiveOffset()` is never called.** It is defined
(`PatternEngine.cpp:189`) and declared (`.h:61`) and that is all — a repo-wide
search finds no caller. `setProgressiveOffset()` *is* called from the
processor, so the offset is configured and then never advanced. The probe
agrees: `E(3,8)%2` and `E(5,13)%5` return the **same pattern on every
trigger**, where `E(1,8)>8` marches. So progressive OFFSET (`%N`, `+N`) looks
non-functional in the engine today, while progressive TRANSFORM (`>N`) works.
Not confirmed in a running host — there may be a path this search missed, and
that check needs ears, not grep.

**`ProgressiveManager` is NOT vestigial — it is mid-migration.** It is
instantiated (`PluginProcessor.cpp:172`) and does real work: progressive state
is saved to and loaded from the ValueTree through it, and cleared through it.
But `applyProgressiveTransformation()` is a **stub returning the base
pattern**, the real transform living in `UPIParser`; and the header carries
`TEMPORARY: Disable ProgressiveManager to isolate legacy system` alongside
`TRANSITION: Use ProgressiveManager if available, fallback to legacy for
safety`. That is an interrupted move from a legacy system, with persistence
already migrated and transformation not. Retiring any of it without finishing
the migration would take state persistence with it.

**Consequence for the JS port.** `@enkerli/upi`'s progressive offset may
currently be the only *working* one in the suite, which is why its rotation
direction has no live C++ behaviour to differential-test against. A
PatternEngine-level harness is easy to write (the class is already linked into
`serpe_parser_probe`: construct, `setPattern`, `setProgressiveOffset`, call
`triggerProgressiveOffset` N times, print) — but it would exercise a function
nothing in the plugin calls, so it would prove the intent, not the behaviour.
The test that would actually settle direction is in-host: type `E(3,8)%2`,
trigger it, and watch which way the onsets walk.
