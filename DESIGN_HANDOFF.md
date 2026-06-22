# Serpe — design handoff

A brief for a **Claude Design** run: redesign **all versions of Serpe** into one
coherent UI that matches the rest of the Enkerli music suite, and that brings
together every feature currently split between the web app and the plugin.

This document is self-contained — you don't need the repo to understand the
scope — but it points to concrete reference files where they help.

---

## 1. What Serpe is

Serpe is the **rhythm stage** of the music suite: a tool for **generating,
transforming, analysing, and playing rhythmic onset patterns**. You describe a
pattern in a compact text notation (or build it with generators), see it on a
circle/step view, analyse its structure (Euclidean evenness, Barlow
indispensability, perfect balance), and play it out as MIDI.

It exists in three forms that must feel like **one product**:

| Version | Runtime | Context | Notes |
|---|---|---|---|
| **Web app** | Browser (desktop + iPad Safari) | Standalone, no DAW | The feature-rich original ("Rhythm Pattern Explorer"). |
| **Desktop plugin** | AU / VST3 / Standalone (macOS) | Inside a DAW | WebView UI; syncs to host transport. |
| **iPad AUv3** | AUv3 + Standalone (iPadOS 26) | Inside AUM etc., touch | WebView UI; **small, touch-first**. |

The plugin UIs are a **JUCE `WebBrowserComponent`** hosting the same React app as
the web app. So: **one React codebase, three deployments.** Sibling plugins
(PitchFold, MIDIcurator, Progression Studio) already work this way — match them.

> Tone note (important): Serpe is a working musician's tool. Copy must be
> **plain, concrete, humble** — no hype, no overpromising, and **never framed
> around "songs."** Talk about patterns, onsets, steps, beats, accents.

---

## 2. The mission of this run

1. **One visual system.** Make Serpe look and feel like the suite (the "paper &
   ink / Sketchbook" aesthetic, §4). Today the web app has its own older look
   and the plugin has a native JUCE UI being retired.
2. **Feature union.** Bring together everything from both the web app and the
   plugin into one UI (§5), respecting which features belong to which runtime.
3. **Three responsive layouts** from one design: roomy desktop, compact iPad
   AUv3, and browser — same components, reflowed.
4. **A proper app icon set** (§7) — the current icon does not appear for the
   AUv3 in hosts on iPadOS 26.

Deliverables: screen designs (light + dark) for each panel at desktop and
iPad-AUv3 widths; the component states; the app-icon set; and notes on the
notation affordances (§6).

---

## 3. Platforms & constraints

- **Single React app**, bundled with esbuild, embedded in the plugin via JUCE
  binary data; served as a normal web app for the browser version. (See
  PitchFold `Source/WebUI/` for the exact structure to mirror.)
- **The JUCE bridge** (`juce-bridge.js`): JS↔C++ is event-based —
  `backend.emitEvent(id, data)` / `backend.addEventListener(id, cb)`. Outside
  JUCE (browser) these are **no-ops**, so the same UI runs standalone. Plugin
  parameters round-trip through a `PARAM_MAP`. Design must not assume host
  presence — every host-only affordance (§5) degrades gracefully in the browser.
- **iPad AUv3 is the tight constraint**: hosts open the plugin UI in a small,
  often short panel, in **portrait or landscape**, touch-only, no hover. Design
  the compact layout **first-class**, not an afterthought — prioritise the
  pattern input + visualiser, make everything else reachable but collapsible.
- **Light + dark** are both required (suite has a theme toggle). Tokens below
  ship both.
- **Accessibility:** WCAG AA contrast (the suite tokens were already lifted to
  meet it — keep that bar). Touch targets ≥ 44px on iPad.

---

## 4. The suite design language to match

Warm-paper **"Sketchbook"** aesthetic — ink on aged paper, restrained, a single
amber accent, four muted "lane" colours for layered patterns. This is shared
suite-wide.

**Canonical token sources to read and reuse (don't reinvent):**
- `~/Desktop/PitchFold/Source/WebUI/design/tokens.jsx` — the React token set
  (PAPER / PAPER_DARK, LANES, SCALES) and `design/ui-primitives.jsx`.
- `~/Desktop/music-suite/packages/ui` — the suite's `--es-*` CSS tokens
  ("paper & ink") used by all the web apps, plus `components.css`.
- **PitchFold is the gold-standard sibling** — its WebUI is the closest target
  for what Serpe's should become. Emulate its structure, density, and feel.

Core palette (light → dark):

```
bg      #F5F0E8 → #1A1A24      card   #FAF8F4 → #22222E
ink     #2D2620 → #DCDCE8      rule   #D4CAB8 → #484862
amber   #C4873A → #E8A838  (single accent)
lanes   ink/blue · rose/amber · moss/green · plum   (for layered patterns)
```

Type: a warm serif for display (Domine) + a clean sans for UI (Inter Tight);
a casual hand (Caveat) is used sparingly for annotations in the suite.

The existing Serpe motif (keep it): the **E(5,8) Euclidean polygon** inscribed
in a circle — see `~/Desktop/music-suite/docs/assets/icons/serpe.svg`.

---

## 5. The complete feature set to bring together

Organised by where each feature lives. **"Paired" = must appear in all
versions.** The split follows the suite's parity rules.

### Paired — in every version (web app + desktop + iPad)
- **Universal Pattern Input (UPI):** one text field that accepts the whole
  notation language (§6) and instantly updates the pattern.
- **Generators / Pattern Creation:**
  - **Euclidean** `E(k,n)` with rotation/offset.
  - **Polygon** patterns (regular k-gons mapped to steps; combinations).
  - **Barlow indispensability** — generate/transform by metric weight.
  - **Random / stochastic** onset generation (bell-curve density control).
  - **Numeric notation:** binary, hex, octal, decimal, onset-array — all
    **leftmost = LSB** (first step = bit 0; step k = 2^k; read left to right).
- **Transformations:** Barlow dilute/concentrate (add/remove onsets by
  indispensability), **Wolrab** (anti-Barlow), **Dilcue** (anti-Euclidean),
  rotate, invert, complement.
- **Progressive patterns:** progressive **offsets** and progressive
  **lengthening** — the pattern evolves step-by-step across cycles.
- **Scenes:** store/recall multiple patterns and step between them.
- **Accents:** accent patterns (their own onset layer), accent velocity vs
  unaccented velocity, accent pitch offset.
- **Analysis:** **perfect-balance** detection and **center-of-gravity**,
  Euclidean evenness, Barlow indispensability tables, onset/step readouts.
- **Visualisation:** the **circle/polygon** view (onsets on a ring, connected)
  **and** a linear **step** view; playhead; accent + lane styling.
- **Quantization**, **subdivision / pattern length**, **swing**.
- **MIDI output config:** output note, channel, the "tick" voice.
- **Theme toggle** (light/dark), **presets**, and concise inline **docs/help**.

### Web-app only (degrade/hide in the plugin)
- **Pattern database** + search, filters, and statistics (browse/collect
  patterns).
- **Web Audio playback** with its own tempo + volume (the plugin uses the host).
- **WebRTC-OSC** output — *candidate for removal*; flag it, don't design around
  it unless asked.

### Plugin only (host-side; absent/disabled in the browser)
- **Host transport sync** (follow the DAW's play/tempo/position) with a
  manual-tempo fallback for Standalone.
- **DAW parameter automation** (the APVTS params: pattern length, subdivision,
  accent velocities, MIDI note, BPM, host-transport toggle) — these should have
  clear UI controls that also reflect automation from the host.
- **MIDI to host** (the generated pattern as MIDI out).

---

## 6. The notation (UPI) — what the input must afford

The text field is central; design should make its grammar **discoverable**
(examples, autocomplete, or a builder that writes the syntax). The language,
in brief:

- `E(k,n)` Euclidean (k onsets in n steps), optional rotation: `E(3,8)`,
  `E(5,8,2)`.
- Polygons, e.g. `P(5,0)`, and **combinations** with `+` / `-` / logical ops.
- Numeric: binary `10010010` (or `b…`), hex `0x49`, octal `0o111`, decimal
  `d73`, onset-array `[0,3,6]`; an optional `:N` step-count suffix preserves
  trailing rests. **All leftmost = LSB** (tresillo `10010010` = `0x49` = 73).
- Barlow/Wolrab/Dilcue, random `R(k,n)`, Morse, …
- **Accents** in `{…}` (e.g. `{10010}E(5,8)`), **progressive** with `>` /
  lengthening, **scenes** with `|`.

(The full grammar lives in `Source/Core/UPIParser.*`; the cross-language
codec convention is in music-suite `CONVENTIONS.md`.)

---

## 7. App icon

The current icon does not render for the **AUv3** in hosts (generic icon shown)
on **iPadOS 26**, even across hosts and reboots — likely an iPadOS-26
icon-format/appex requirement rather than caching. Please produce a **complete,
current-spec app-icon set** for both the standalone app and the AUv3 extension:

- Built on the existing Serpe motif (E(5,8) polygon on a paper card) so it sits
  in the suite's icon family alongside PitchFold/Vane/etc.
- Whatever sizes/format iPadOS 26 + current Xcode require (single high-res
  master + any layered/“tinted” variants the OS now expects), plus macOS.
- A 1024² master PNG and the source.

---

## 8. Guardrails / done-criteria

- Looks unmistakably part of the suite (PitchFold as the bar).
- Every **paired** feature present and usable at **iPad-AUv3 width**, not just
  desktop.
- Host-only features degrade cleanly in the browser; web-only features hidden in
  the plugin.
- Light + dark; WCAG AA; ≥44px touch targets.
- Copy is plain and concrete; no hype; not "songs."
- The app icon shows in AUv3 hosts on iPadOS 26.

### Reference files
- Sibling to emulate: `~/Desktop/PitchFold/Source/WebUI/` (esp. `design/`,
  `main.jsx`, `juce-bridge.js`).
- Suite tokens: `~/Desktop/music-suite/packages/ui`.
- Serpe's current web app (feature reference): `WebApp/app/` (notably
  `pattern-analysis.js`, `pattern-processing.js`, `sequencer-visual.js`,
  `pattern-exploration.js`).
- Notation + conventions: `Source/Core/UPIParser.*`, music-suite `CONVENTIONS.md`.
- Icon motif: `~/Desktop/music-suite/docs/assets/icons/serpe.svg`.
