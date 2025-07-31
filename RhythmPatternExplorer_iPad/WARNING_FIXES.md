# Warning Fixes for Clean Template

## 1. Fix UIScene Lifecycle Warning

**In Projucer iOS Exporter settings:**
- Add `UISceneDelegate` support
- Or update to JUCE 8.0.2+ which handles this automatically

**Manual fix in Info.plist:**
```xml
<key>UIApplicationSceneManifest</key>
<dict>
    <key>UIApplicationSupportsMultipleScenes</key>
    <false/>
    <key>UISceneConfigurations</key>
    <dict>
        <key>UIWindowSceneSessionRoleApplication</key>
        <array>
            <dict>
                <key>UISceneConfigurationName</key>
                <string>Default Configuration</string>
                <key>UISceneDelegateClassName</key>
                <string>SceneDelegate</string>
            </dict>
        </array>
    </dict>
</dict>
```

## 2. Suppress CoreMIDI System Messages

**Option A: Accept them as normal**
- These are iOS system messages
- Don't indicate plugin problems
- Common in all iOS audio apps

**Option B: Reduce MIDI scanning**
Add to app's Info.plist:
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app uses MIDI connections</string>
<key>NSLocalNetworkUsageDescription</key>
<string>This app discovers MIDI devices on local network</string>
```

## 3. Update JUCE Version (Recommended)

**Best solution**: Update to JUCE 8.0.2+ which:
- Fixes UIScene warnings automatically
- Reduces CoreMIDI system messages
- Has better iOS 17+ compatibility

## For Template Release

Since these are harmless system messages, we can:
1. Document them as "normal and harmless"
2. Provide fixes for users who want cleaner console
3. Note that functionality is unaffected

Most professional iOS audio apps have similar messages.