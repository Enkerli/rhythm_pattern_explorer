# CoreMIDI Error Troubleshooting

## Common CoreMIDI Errors in iPad AUv3 Plugins

### 1. **"MIDIClientCreate failed" or similar**

**Cause**: App doesn't have proper MIDI permissions or CoreMIDI setup

**Fix**: Add to Info.plist (main app target):
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app uses MIDI over Bluetooth</string>
<key>NSLocalNetworkUsageDescription</key>
<string>This app uses MIDI over network</string>
```

### 2. **"kMIDIDriverPropertyNotSupported"**

**Cause**: iOS simulator CoreMIDI limitations

**Fix**: Test on actual iPad device, not simulator

### 3. **"MIDI endpoint not found"**

**Cause**: AUv3 extension trying to create MIDI clients directly

**Fix**: Let the host handle MIDI connections. AUv3 extensions should use:
- `MidiBuffer` in `processBlock()` for I/O
- NOT create `MIDIClient` directly

### 4. **Background MIDI errors**

**Cause**: Missing background audio capability

**Fix**: Verify in Xcode target settings:
- Signing & Capabilities → Background Modes → Audio

### 5. **"OSStatus error -50" (paramErr)**

**Cause**: Invalid MIDI parameters

**Fix**: Check MIDI message creation:
```cpp
// Ensure valid ranges
noteNumber = juce::jlimit(0, 127, noteNumber);
velocity = juce::jlimit(0, 127, velocity);
channel = juce::jlimit(1, 16, channel);
```

## Quick Diagnostic

1. **Run on iPad** (not simulator)
2. **Check console** for specific error codes
3. **Test in AUM** vs standalone
4. **Verify MIDI flows** through host connections

## Template-Specific Notes

Our template uses `MidiMessageCollector` which is the correct approach for AUv3 extensions. CoreMIDI errors might be:
- Host-related (not our plugin)
- iOS system messages (usually harmless)
- Network/Bluetooth MIDI discovery (can ignore)

Most CoreMIDI errors in AUv3 context are informational and don't affect functionality.