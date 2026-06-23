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

## The fix — now durable in CMake (was a manual Xcode step)

The artwork master lives at `Assets/Serpe.xcassets/SerpeAppIcon.appiconset` — a
single-1024 App Icon set with the three appearances: **Any** = `icon_any.png`
(`serpe-1024-bleed.png`), **Dark** = `icon_dark.png`, **Tinted** =
`icon_tinted.png`.

JUCE already generates its own single-appearance `AppIcon.appiconset` (from
`ICON_BIG`) into each format target's `JuceLibraryCode/.../Images.xcassets`, and
`actool` runs on THAT catalog. Adding a *second* catalog fails — `actool` then
can't find the second app-icon ("None of the input catalogs contained … named
…") and merging two catalogs on one target is fragile.

So `CMakeLists.txt` (iOS branch) instead **overwrites JUCE's generated
`AppIcon.appiconset`** with our appearance version (same name, after juceaide
has generated it). Result: exactly ONE catalog with ONE `AppIcon`, the default
`ASSETCATALOG_COMPILER_APPICON_NAME=AppIcon` still applies, and the appex's
`AppIcon` now carries the appearances. Re-applied on every configure, so it
survives JUCE regenerating the catalog.

Verified on the build host (no device/signing needed): after configure,
`xcrun actool <Serpe_AUv3 Images.xcassets> --app-icon AppIcon …` compiles a
`Assets.car` whose `AppIcon` carries `UIAppearanceAny` / `UIAppearanceDark` /
`ISAppearanceTintable` — **with no errors** (the earlier two-catalog
"None of the input catalogs …" failure is gone).

**Still needs the maintainer's signed iPad build + AUM check** to confirm it
displays on-device, and that the Xcode build doesn't regenerate JUCE's catalog
*during* the build (overwriting the configure-time copy). If the icon is still
blank, strip JUCE's legacy `CFBundleIcons` keys from the appex `Info.plist`.
The same wiring belongs in `enkerli-juce` eventually so every suite plugin
inherits it.
