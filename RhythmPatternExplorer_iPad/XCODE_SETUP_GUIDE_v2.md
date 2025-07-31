# iPad AUv3 Plugin - Xcode Setup Guide v2 (Fixed)

## Issues Resolved:
- ✅ Fixed corrupted .jucer file formatting
- ✅ Removed corrupted build directory
- ✅ Added proper plugin characteristics configuration
- ✅ Simplified iOS exporter settings

## Step 1: Generate Xcode Project (Retry)

1. **Open Projucer**
2. **Open** `RhythmPatternExplorer_iPad.jucer`
3. **Verify Critical Settings**:
   - **Project Type**: Audio Plug-in ✅
   - **Plugin is a MIDI Effect**: YES ✅
   - **Plugin produces MIDI output**: YES ✅
   - **Plugin wants MIDI input**: YES ✅
   - **Plugin is a synth**: NO ❌
   - **Bundle ID**: `com.alex.rhythmpatternexploreripad` ✅

4. **Check Exporter Settings**:
   - iOS Deployment Target: 15.0 ✅
   - Build AUv3: YES ✅
   - Build AU: NO ❌
   - Build VST/VST3: NO ❌

5. **Click "Save and open in IDE"**

## Step 2: If Xcode Still Won't Open

### Option A: Generate Manually
If Projucer still fails:
1. In Projucer, just click **"Save Project"** (don't open in IDE)
2. Close Projucer
3. Navigate to: `/Users/alex/Documents/Coding/rhythm_pattern_explorer/RhythmPatternExplorer_iPad/Builds/iOS/`
4. Double-click `Rhythm Pattern Explorer iPad.xcodeproj`

### Option B: Clean Generation
If still corrupted:
1. **Close Projucer completely**
2. **Delete Builds folder**: `rm -rf Builds/`
3. **Reopen Projucer**
4. **Open .jucer file again**
5. **Try "Save and open in IDE" again**

## Step 3: Xcode Project Structure (Expected)

Once Xcode opens successfully, you should see:

```
Rhythm Pattern Explorer iPad (Project)
├── Rhythm Pattern Explorer iPad (App Target)
│   ├── Source/
│   │   ├── PluginProcessor.cpp
│   │   ├── PluginProcessor.h
│   │   ├── PluginEditor.cpp
│   │   └── PluginEditor.h
│   └── Resources/
└── AUv3 (Extension Target)
    └── [Generated AUv3 files]
```

## Step 4: Critical Configuration Fixes

### A. Fix App Target Settings
**Select "Rhythm Pattern Explorer iPad" target:**
1. **General Tab**:
   - Deployment Target: iOS 15.0+
   - Devices: Universal (iPhone + iPad)
   - Bundle Identifier: `com.alex.rhythmpatternexploreripad`

2. **Signing & Capabilities**:
   - Team: [Your Apple Developer Team]
   - Add Capability: **App Sandbox**
   - Add Capability: **Background Modes** → **Audio**

### B. Fix AUv3 Extension Target
**Select "AUv3" target:**
1. **General Tab**:
   - Bundle Identifier: `com.alex.rhythmpatternexploreripad.AUv3`
   - Deployment Target: iOS 15.0+

2. **Signing & Capabilities**:
   - Team: [Your Apple Developer Team]  
   - Add Capability: **App Sandbox**
   - Add Capability: **Audio Unit Extension**

### C. Configure AUv3 Info.plist

**Critical**: The AUv3 extension needs proper Info.plist settings.

1. **Find AUv3 target's Info.plist** (usually in `Builds/iOS/`)
2. **Add/verify this NSExtension block**:

```xml
<key>NSExtension</key>
<dict>
    <key>NSExtensionAttributes</key>
    <dict>
        <key>AudioComponentDescription</key>
        <dict>
            <key>componentType</key>
            <string>aurm</string>
            <key>componentSubType</key>
            <string>RPEi</string>
            <key>componentManufacturer</key>
            <string>Alex</string>
            <key>componentFlags</key>
            <integer>0</integer>
            <key>componentFlagsMask</key>
            <integer>0</integer>
        </dict>
    </dict>
    <key>NSExtensionPointIdentifier</key>
    <string>com.apple.AudioUnit</string>
    <key>NSExtensionPrincipalClass</key>
    <string>AUv3_AUAudioUnit</string>
</dict>
```

## Step 5: Test Build

1. **Connect iPad to Mac**
2. **Select iPad as target device**
3. **Build App target** (Cmd+B)
4. **If build succeeds**: Run on device (Cmd+R)

## Step 6: Common Build Issues & Solutions

### "No provisioning profile"
- **Solution**: Add your Apple Developer Team in Signing & Capabilities

### "Invalid bundle identifier"
- **Solution**: Change bundle ID to something unique to you: `com.yourname.rhythmpatternexploreripad`

### "AUv3 extension not found"
- **Solution**: Verify Info.plist NSExtension block is correct
- **Check**: componentSubType matches pluginCode ("RPEi")
- **Check**: componentManufacturer matches pluginManufacturerCode ("Alex")

### "Build failed with audio unit errors"
- **Solution**: Clean build (Cmd+Shift+K), then rebuild
- **Check**: Both targets have same deployment target (15.0+)

### "App installs but AUv3 not visible in host apps"
- **Solution**: Verify componentType is "aurm" (MIDI effect)
- **Check**: Host app is looking in MIDI Effects, not Instruments
- **Try**: Restart host app after installing plugin

## Step 7: Testing in Host Apps

Once build succeeds and app runs on iPad:

1. **Install AUM** (or use Logic Pro for iPad)
2. **Create new project**
3. **Add MIDI track**
4. **Add MIDI Effect** → Look for "Rhythm Pattern Explorer iPad"
5. **Load plugin** → Tap button to send MIDI note
6. **Verify**: MIDI note appears in host's MIDI monitor

## Troubleshooting Checklist

- [ ] .jucer file opens in Projucer without errors
- [ ] Xcode project opens without "damaged" error
- [ ] Both app and AUv3 targets have valid bundle IDs
- [ ] Both targets have Apple Developer Team selected
- [ ] AUv3 Info.plist has correct NSExtension block
- [ ] Build succeeds for both targets
- [ ] App runs on iPad device
- [ ] Plugin appears in AUM/Logic Pro under MIDI Effects

## Next Steps After Success

Once the basic plugin works:
1. Test MIDI output in host app
2. Verify UI responsiveness on iPad
3. Add pattern generation functionality
4. Port desktop plugin features gradually

Let me know which step fails and I'll help debug further!