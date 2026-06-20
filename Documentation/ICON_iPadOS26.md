# Serpe app icon — and the iPadOS 26 AUv3 fix

The artwork is done and in the repo. The remaining piece is a **packaging**
step on the AUv3 that needs the maintainer's Xcode/iPad to apply and verify.

## What's in the repo

`Assets/icon/` — the full set from the design handoff (DESIGN_HANDOFF §7):

- SVG masters: `serpe-icon.svg` (rounded paper card), `serpe-icon-dark.svg`,
  `serpe-icon-tinted.svg` (grayscale-on-transparent for the iOS tinted
  appearance), `serpe-icon-bleed.svg` (full-bleed square).
- PNGs at every size, incl. `serpe-1024-bleed.png`, `serpe-1024-dark.png`,
  `serpe-1024-tinted.png`, and the iOS ramp `serpe-ios-{40…180}.png`.

Motif: the `E(5,8)` Euclidean onset polygon in a step ring, suite "paper & ink",
Serpe's moss-green accent. `CMakeLists.txt` passes `serpe-1024-bleed.png` as
`ICON_BIG` (the OS applies the corner mask). Verified: the macOS build emits a
1024² `AppIcon.icns` into the bundle.

## Why the AUv3 icon was blank on iPadOS 26

Not the artwork and not caching (reboots were already ruled out). On iPadOS 26
the **app-extension** icon must come from a *compiled asset catalog* on the
appex target — loose `CFBundleIcons` / `CFBundleIconFiles` in the appex
`Info.plist` are no longer honoured. JUCE's generated single-appearance
appiconset (from `ICON_BIG`) doesn't satisfy this for the appex.

## The fix (Xcode, on the iOS build)

After `cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS`, in the generated
project, on the **`Serpe_AUv3` appex target** (not just the standalone):

1. Add an `Assets.xcassets` with an **App Icon** set that has the three
   appearances: **Any** = `serpe-1024-bleed.png`, **Dark** =
   `serpe-1024-dark.png`, **Tinted** = `serpe-1024-tinted.png` (single 1024 well
   each; let Xcode derive the rest).
2. Set the appex's `ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon` and remove any
   legacy `CFBundleIcons` keys.
3. Clean build folder so `actool` recompiles; verify with
   `assetutil --info <appex>/Assets.car` that the icon is present.

This crosses into the build because the regenerated CMake project is overwritten
on reconfigure — so to make it durable it should move into the CMake/`enkerli-juce`
layer (inject a per-appex catalog) rather than be hand-set in Xcode each time.
Tracked as a follow-up; the artwork above is the input either way.
