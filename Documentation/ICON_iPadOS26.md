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

Implemented in `CMakeLists.txt` (the iOS branch), so it survives reconfigure:

- `Assets/Serpe.xcassets/SerpeAppIcon.appiconset` — a single-1024 App Icon set
  with the three appearances: **Any** = `icon_any.png` (`serpe-1024-bleed.png`),
  **Dark** = `icon_dark.png`, **Tinted** = `icon_tinted.png`.
- On the `Serpe_AUv3` appex (and `Serpe_Standalone`): the catalog is added to the
  target and `XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME = SerpeAppIcon`.
  The set is named **`SerpeAppIcon`** (distinct from JUCE's generated `AppIcon`)
  so the two catalogs on one target don't clash; `actool` compiles ours as the
  app icon and ignores JUCE's.

Verified as far as possible on the build host (no device/signing needed):
`xcrun actool Assets/Serpe.xcassets --app-icon SerpeAppIcon …` compiles a
`Assets.car` whose `SerpeAppIcon` carries `UIAppearanceAny` / `UIAppearanceDark`
/ `ISAppearanceTintable`, and emits `CFBundleIconName = SerpeAppIcon`
(iPhone + ~ipad). The iOS project also configures cleanly with the catalog
attached and the setting applied.

**Still needs the maintainer's signed iPad build + AUM check** to confirm it
displays on-device. If the icon is still blank, the documented fallback is to
strip JUCE's legacy `CFBundleIcons` keys from the appex `Info.plist` (the
asset-catalog `CFBundleIconName` should already win). The same wiring belongs in
`enkerli-juce` eventually so every suite plugin inherits it.
