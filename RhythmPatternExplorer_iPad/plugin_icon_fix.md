# AUv3 Plugin Icon Fix for DAW Hosts

## The Problem
AUv3 plugin icons don't show in DAW hosts because they use the main app icon bundle, not a separate plugin icon.

## The Solution 
For iOS AUv3 plugins, the plugin icon is actually **the same as the app icon**. DAW hosts read the icon from the app bundle's icon set.

## Why This Happens
- **Desktop plugins** (VST/AU) can have separate plugin icons
- **iOS AUv3 plugins** inherit their icon from the app's `AppIcon.appiconset`
- DAW hosts like Logic, GarageBand, AUM use the app icon for the plugin

## Steps to Fix

### 1. Ensure App Icons Are Generated
- Use the icon generator to create all app icon sizes
- Place them in: `NewProject/Builds/iOS/[Target]/Images.xcassets/AppIcon.appiconset/`
- Replace all placeholder icon files

### 2. Verify Icon Names Match Contents.json
Check that the downloaded icon filenames exactly match the entries in `Contents.json`:
```json
{
  "filename": "Icon-AppStore-1024.png",
  "size": "1024x1024",
  ...
}
```

### 3. Clean Build
- Clean build folder in Xcode
- Rebuild the project
- Reinstall to device

### 4. Test in Multiple DAW Hosts
- **Logic Pro**: Should show 64px icon
- **GarageBand**: Should show 32px icon  
- **AUM**: Should show 64px/128px icon
- **Cubasis**: Should show 128px icon

## Alternative: Check Host-Specific Issues
Some DAW hosts cache plugin icons. Try:
1. Delete and reinstall the plugin
2. Restart the DAW host app
3. Clear DAW host's plugin cache (if available)

## Expected Result
Once the app icons are properly installed, DAW hosts should display the donut pattern icon for your AUv3 plugin.