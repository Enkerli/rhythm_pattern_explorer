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

- [ ] **M1 — foundation submodule + plan** (this doc + `enkerli-juce` submodule).
- [ ] **M2 — flatten + CMakeLists** mirroring PitchFold via
      `enkerli_add_midi_effect_plugin(Serpe PLUGIN_CODE RPEd BUNDLE_ID com.enkerli.serpe …)`.
      Source list = existing `Core`/`Managers`/`Platform` (sans the native editor).
- [ ] **M3 — React WebUI scaffold** under `Source/WebUI` (package.json, esbuild,
      `index.html`, `juce-bridge.js`, `react-globals.js`, `design/` tokens+primitives,
      `main.jsx`). Bundles + renders in a browser. Minimal shell wired to the bridge;
      real panels filled in during the design-led build-out.
- [ ] **M4 — WebView editor + processor swap.** `Source/WebUI/SerpeEditor.{h,cpp}`
      (WebBrowserComponent serving embedded BinaryData, native integration on);
      `PluginProcessor::createEditor()` returns it; add the JS↔C++ bridge
      (parameter relay, pattern in/out). Retire native `PluginEditor`.
- [ ] **M5 — re-home conformance test** from the `.jucer` post-build to a CTest /
      CMake custom target so `ctest` (and CI) runs it.
- [ ] **M6 — feature pairing** (Phase 2): per the parity rules — OSC/audio
      webapp-only, host-sync plugin-only, everything else paired.

## Verification ladder

- Web parts (bundle builds, React renders, bridge messages) — verifiable in a
  browser without JUCE.
- `cmake -B build-macos` configure — verifiable locally (JUCE at /Applications/JUCE
  or fetched).
- Full plugin build (AU/VST3/Standalone, iOS AUv3) + auval + host smoke test —
  the maintainer's Xcode/signing workflow. Checkpoint here each milestone.

See the `serpe-webapp-plugin-parity` memory and CONVENTIONS.md (music-suite).
