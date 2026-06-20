# Serpe → CMake + WebView migration

Adapt Serpe to the suite's modern foundation (`enkerli-juce` CMake archetypes +
React WebView UI), the same one PitchFold / midicurator-plugin /
progression-studio-plugin use. Decided 2026-06-20 (over rebuilding from scratch:
Serpe's ~7.6k LOC engine has **zero UI coupling**, so it ports as-is — only the
build system and the UI shell change).

## Decisions

- **Adapt, not rebuild.** Keep `Core/` (UPIParser notation engine, PatternEngine,
  PatternUtils, QuantizationEngine, AccentSequence), `Managers/` (Preset, Scene,
  Progressive), and `PluginProcessor`. They have no GUI includes. Rebuilding would
  re-incur the timing/accent/notation bugs already fixed.
- **Flatten** the repo to PitchFold's layout (`Source/` at root, root
  `CMakeLists.txt`, `enkerli-juce/` submodule, `build-macos`/`build-ios`). Retire
  `Serpe/Serpe.jucer`, `Serpe/Builds`, `Serpe/JuceLibraryCode`, `Serpe-iOS/`.
- **Straight to React** design system for the WebView UI (not the vanilla webapp
  as an interim). The existing `WebApp/` stays as the conformance-tested reference
  and the functional spec for the React port; UI build-out is design-led (Claude
  Design) and happens panel by panel.
- **Plugin identity is load-bearing — do not change.** `PLUGIN_CODE` `RPEd`
  (desktop), manufacturer `Enke`, `BUNDLE_ID com.enkerli.serpe`. Changing these
  orphans saved host sessions. (Archetype default would lowercase the bundle id —
  pass it explicitly.)

## Milestones

- [x] **M1 — foundation submodule + plan** (this doc + `enkerli-juce` submodule).
- [x] **M2 — flatten + CMakeLists** via `enkerli_add_midi_effect_plugin(Serpe
      PLUGIN_CODE RPEd BUNDLE_ID com.enkerli.serpe …)`. Builds the *current*
      plugin (native UI intact) — pure build-system swap, no behaviour change.
      Verified locally: `cmake -B build-macos` configures, `Serpe_Standalone`
      **compiles + links** (macOS), `serpe_conformance` passes via `ctest`.
      Needed `SERPE_DESKTOP=1` (the `.jucer` set it). Retired `.jucer`/`Builds`/
      `JuceLibraryCode`/`Serpe-iOS`. **M5 folded in**: conformance re-homed to a
      `juce_add_console_app` CTest target + plugin POST_BUILD (the old
      `run-conformance.sh` + `Colour` stub are gone). AU/VST3 + iOS = maintainer build.
- [x] **Design run:** Claude Design delivered `Serpe - Suite Redesign.html` +
      `Serpe - App Icon.html` (handoff bundle, gitignored). Full unified UI on the
      suite paper & ink tokens, three responsive runtimes, feature union, icon set.
- [x] **M3 — React WebUI** under `Source/WebUI` (esbuild, mirrors PitchFold).
      Implements the redesign: top bar/transport, UPI input + live parse, circle/step
      visualisers, full control rail with runtime feature-gating. Reuses the design's
      framework-agnostic engine (`engine/*.js`) + suite CSS. Verified in-browser:
      builds, pixel-accurate light+dark, correct engine math, no console errors.
- [x] **M4 — WebView editor + processor swap.** `Source/WebUI/SerpeEditor.{h,cpp}`
      (WebBrowserComponent serving embedded BinaryData). `createEditor()` returns it;
      bridge: JS→C++ `setUPI`/`setParamActual`, C++→JS `stateSnapshot`/`paramChange`/
      `transport`. Native `PluginEditor` dropped from the build. CMake bundles the
      WebUI (esbuild) + embeds it. Verified: builds, **auval PASS** with the editor.
- [x] **M5 — conformance via CTest** (folded into M2).
- [~] **Icon:** artwork done — full set in `Assets/icon`, `ICON_BIG` =
      `serpe-1024-bleed.png` (macOS verified). iPadOS-26 AUv3 packaging fix
      documented in `Documentation/ICON_iPadOS26.md` (needs maintainer Xcode/iPad).
- [ ] **M6 — feature pairing** (Phase 2): per the parity rules — OSC/audio
      webapp-only, host-sync plugin-only, everything else paired. New UI controls
      not yet backed by APVTS params (swing, MIDI channel) are UI-local until added.

## Verification ladder

- Web parts (bundle builds, React renders, bridge messages) — verifiable in a
  browser without JUCE.
- `cmake -B build-macos` configure — verifiable locally (JUCE at /Applications/JUCE
  or fetched).
- Full plugin build (AU/VST3/Standalone, iOS AUv3) + auval + host smoke test —
  the maintainer's Xcode/signing workflow. Checkpoint here each milestone.

See the `serpe-webapp-plugin-parity` memory and CONVENTIONS.md (music-suite).
