# Xcode Project Configuration Steps

## Step 1: Replace Source Files

### Current State:
The project currently references the complex desktop plugin source files. We need to replace them with our minimal iPad versions.

### In Xcode:

**A. Remove Old Source References:**
1. In the Navigator, expand the "Source" group
2. Select all the complex source files (PatternEngine.cpp, UPIParser.cpp, etc.)
3. **Right-click → Delete** → **"Remove References Only"** (don't move to trash)

**B. Add Our Minimal Source Files:**
1. **Right-click on "Source" group** → **"Add Files to 'Rhythm Pattern Explorer'"**
2. Navigate to: `/Users/alex/Documents/Coding/rhythm_pattern_explorer/RhythmPatternExplorer_iPad/Source/`
3. **Select all 4 files**:
   - `PluginProcessor.cpp`
   - `PluginProcessor.h`
   - `PluginEditor.cpp`
   - `PluginEditor.h`
4. **Click "Add"**

## Step 2: Update Bundle Identifiers

### A. Main App Target:
1. **Select project** in Navigator (top level)
2. **Select "Rhythm Pattern Explorer" target** (app icon)
3. **General tab**:
   - Bundle Identifier: `com.enkerli.rhythmpatternexploreripad`
   - Display Name: `Rhythm Pattern Explorer iPad`
   - Version: `0.01`

### B. AUv3 Extension Target:
1. **Select "AUv3" target** (puzzle piece icon)
2. **General tab**:
   - Bundle Identifier: `com.enkerli.rhythmpatternexploreripad.AUv3`
   - Display Name: `Rhythm Pattern Explorer iPad`
   - Version: `0.01`

## Step 3: Update AUv3 Component Description

### Find and Edit Info-AUv3_AppExtension.plist:
1. In Navigator, look for `Info-AUv3_AppExtension.plist`
2. **Double-click to open**
3. **Find the AudioComponentDescription section**
4. **Update these values**:

```xml
<key>AudioComponentDescription</key>
<dict>
    <key>componentType</key>
    <string>aurm</string>
    <key>componentSubType</key>
    <string>RPEi</string>  <!-- Change from "RhPx" to "RPEi" -->
    <key>componentManufacturer</key>
    <string>Enke</string>  <!-- Change from "RhPt" to "Enke" -->
    <key>componentFlags</key>
    <integer>0</integer>
    <key>componentFlagsMask</key>
    <integer>0</integer>
</dict>
```

## Step 4: Update Signing & Capabilities

### For Both Targets:
1. **Select target** → **Signing & Capabilities tab**
2. **Team**: Select your Apple Developer Team
3. **Automatically manage signing**: ✅ Checked
4. **Verify capabilities are present**:
   - ✅ App Sandbox
   - ✅ Background Modes (Audio) 
   - ✅ Audio Unit Extension (AUv3 target only)

## Step 5: Test Build

### Build Both Targets:
1. **Select iPad device** (or iPad simulator) as target
2. **Product → Build** (Cmd+B)
3. **Verify no compile errors**

### Expected Results:
- ✅ Both targets compile successfully
- ✅ No missing file errors
- ✅ Clean build output

## Step 6: Test Run

1. **Select main app target**
2. **Product → Run** (Cmd+R)
3. **App should install and launch on iPad**
4. **Basic UI should appear with "Send MIDI Note" button**

## Next Steps After Success:
- Test the plugin in AUM or Logic Pro for iPad
- Verify MIDI output functionality
- Begin porting more features from desktop version

---

## Troubleshooting:

**"Missing files" errors**: Make sure you removed references, not deleted files
**"Duplicate symbols" errors**: Ensure old complex source files are fully removed
**"Code signing" errors**: Set your Apple Developer Team in both targets
**"Bundle ID" errors**: Make sure both bundle IDs are unique and properly formatted

Let me know when you complete each step!