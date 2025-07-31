# iPad AUv3 Plugin - Xcode Setup Guide

## Step 1: Generate Xcode Project from Projucer

1. **Open Projucer**
2. **Open** `RhythmPatternExplorer_iPad.jucer`
3. **Verify Settings**:
   - Plugin Type: Audio Plug-in
   - Plugin is a MIDI Effect: ✅ YES
   - Plugin produces MIDI output: ✅ YES
   - Plugin wants MIDI input: ✅ YES
   - Plugin is a synth: ❌ NO
4. **Click "Save and open in IDE"**

## Step 2: Configure Xcode Project

Once Xcode opens, you'll see two targets:
- **RhythmPatternExplorer_iPad** (Container App)
- **RhythmPatternExplorer_iPad AUv3** (Plugin Extension)

### A. Configure AUv3 Extension Target

1. **Select the AUv3 extension target**
2. **Go to Info tab**
3. **Find the Info.plist file for the extension**
4. **Add the NSExtension section** (use the template provided in `Info.plist.template`)

### B. Key Info.plist Values to Verify:
```xml
<key>NSExtension</key>
<dict>
    <key>NSExtensionAttributes</key>
    <dict>
        <key>AudioComponentDescription</key>
        <dict>
            <key>componentType</key>
            <string>aurm</string>          <!-- MIDI Effect -->
            <key>componentSubType</key>
            <string>RPEi</string>          <!-- Must match .jucer pluginCode -->
            <key>componentManufacturer</key>
            <string>Alex</string>          <!-- Must match .jucer pluginManufacturerCode -->
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

### C. Configure Container App Target

1. **Select the main app target**
2. **General Tab**:
   - Bundle Identifier: `com.alex.rhythmpatternexploreripad`
   - Deployment Target: iOS 15.0 or higher
   - Device: Universal (iPhone + iPad)

### D. Signing & Capabilities

**For both targets:**
1. **Signing & Capabilities tab**
2. **Team**: Select your Apple Developer Team
3. **Add Capabilities**:
   - ✅ App Sandbox (if not already present)
   - ✅ Audio Unit Extension (for AUv3 target only)
   - ✅ Background Modes → Audio (for both targets)

## Step 3: Build Settings Verification

### Important Build Settings for iOS:
- **Architectures**: arm64 only
- **Build Active Architecture Only**: NO
- **iOS Deployment Target**: 15.0+
- **Code Signing**: Automatic (with your developer team)

## Step 4: Test Build

1. **Connect your iPad**
2. **Select iPad as target device**
3. **Build the container app** (Cmd+B)
4. **Run on device** (Cmd+R)

## Step 5: Verify AUv3 Registration

When the app runs on iPad:
1. The container app should launch successfully
2. The AUv3 plugin will be registered with the system
3. You can now find it in host apps like AUM, Logic Pro for iPad, etc.

## Common Issues & Solutions

### Build Errors:
- **"No code signature"**: Set up your developer team in signing
- **"Invalid Info.plist"**: Check the NSExtension dictionary format
- **"Missing entitlements"**: Add Audio Unit Extension capability

### Runtime Issues:
- **Plugin not found in host**: Check componentType is "aurm" for MIDI effect
- **Plugin crashes on load**: Verify NSExtensionPrincipalClass is correct

### Testing in Host Apps:
1. **AUM**: Look under MIDI Effects → Your Plugin Name
2. **Logic Pro iPad**: Add MIDI Effect → Your Plugin Name
3. **GarageBand**: MIDI Effects section

## Next Steps After Basic Setup

Once this minimal version works:
1. Test MIDI note sending in a host app
2. Gradually port features from the desktop version
3. Add touch-optimized UI elements
4. Implement pattern engine functionality

## File Structure After Xcode Generation:
```
RhythmPatternExplorer_iPad/
├── RhythmPatternExplorer_iPad.jucer
├── Source/
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h  
│   ├── PluginEditor.cpp
│   └── PluginEditor.h
├── Builds/iOS/
│   └── RhythmPatternExplorer_iPad.xcodeproj
└── [Generated Xcode files]
```