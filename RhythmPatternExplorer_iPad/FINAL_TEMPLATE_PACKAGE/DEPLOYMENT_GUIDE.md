# iPad AUv3 MIDI Plugin Deployment Guide

This guide covers the essential steps for deploying your iPad AUv3 MIDI plugin to your device and distributing it to users.

## Table of Contents
- [Development Setup](#development-setup)
- [Building for iPad](#building-for-ipad)
- [Installing on Device](#installing-on-device)
- [Testing in Host Apps](#testing-in-host-apps)
- [Common Issues and Solutions](#common-issues-and-solutions)
- [Distribution](#distribution)

## Development Setup

### 1. Apple Developer Account
- **Required**: Paid Apple Developer account ($99/year)
- **Purpose**: Code signing and device registration
- **Setup**: Register at [developer.apple.com](https://developer.apple.com)

### 2. Development Certificates
1. Open Xcode → Preferences → Accounts
2. Add your Apple ID and download certificates
3. Xcode will automatically manage provisioning profiles

### 3. Device Registration
1. Connect your iPad via USB
2. In Xcode: Window → Devices and Simulators
3. Register your device for development

## Building for iPad

### 1. Configure Bundle Identifier
**CRITICAL**: Your bundle identifier must follow this pattern:
```
Main app: com.yourcompany.yourplugin
AUv3 extension: com.yourcompany.yourplugin.AUv3
```

⚠️ **Common mistake**: Using underscores or missing the `.AUv3` suffix will cause installation failures.

### 2. Set Development Team
1. Open your `.xcodeproj` file in Xcode
2. Select the main project in navigator
3. For BOTH targets (app and AUv3 extension):
   - Go to Signing & Capabilities
   - Select your development team
   - Ensure "Automatically manage signing" is checked

### 3. Build Settings
- **Deployment Target**: iOS 15.0 or later
- **Architecture**: arm64 (required for modern iPads)
- **Build Configuration**: Use "Release" for final testing

### 4. Build the Project
```bash
# In Xcode, select your iPad as target device
# Product → Build (⌘+B)
# Or use xcodebuild from command line:
xcodebuild -scheme "YourPlugin - AUv3" -configuration Release -destination "platform=iOS,name=YourIPadName"
```

## Installing on Device

### 1. Direct Installation via Xcode
1. Connect iPad via USB
2. Select iPad as destination in Xcode
3. Product → Run (⌘+R)

### 2. Enable Development Profile on iPad
After installation, if you see "Untrusted Developer":

1. **iPad Settings** → **General** → **VPN & Device Management**
2. Find your developer profile under "Developer App"
3. Tap your profile → **Trust "[Your Developer Name]"**
4. Confirm by tapping **Trust** again

### 3. Verify Installation
1. Check that the plugin appears in host apps (see next section)
2. If not visible, try restarting the host app
3. Check iOS Settings → General → About → Certificates for your profile

## Testing in Host Apps

### Recommended Host Apps
- **AUM (Audio Mixer)** - Best for initial testing
- **Logic Pro for iPad** - Professional testing environment
- **Loopy Pro** - Loop-based testing
- **Cubasis** - Multi-track testing

### Testing Checklist
- [ ] Plugin appears in host's plugin list
- [ ] Plugin loads without crashes
- [ ] UI displays correctly in portrait/landscape
- [ ] MIDI input/output functions properly
- [ ] Parameters save/restore correctly
- [ ] Plugin survives host app backgrounding

## Common Issues and Solutions

### "Unable to Install" Error
**Problem**: `does not have expected identifier prefix`

**Solution**: 
1. Check bundle identifier follows the pattern: `com.company.plugin.AUv3`
2. Ensure both app and extension use same base identifier
3. Use dots (.) not underscores (_) in bundle ID

### Plugin Doesn't Appear in Hosts
**Causes & Solutions**:
1. **AUv3 not properly registered**
   - Restart the iPad
   - Re-install the plugin
   
2. **Bundle ID mismatch**
   - Verify `.AUv3` suffix on extension
   
3. **Development profile not trusted**
   - Follow "Enable Development Profile" steps above

### Build Failures
**Common Issues**:

1. **"No matching provisioning profiles found"**
   - Check Apple Developer account status
   - Refresh provisioning profiles in Xcode

2. **"Code signing error"**
   - Verify development team is selected
   - Check certificate validity in Keychain Access

3. **"Unsupported architecture"**
   - Ensure Valid Architectures includes `arm64`
   - Remove simulator architectures for device builds

### Runtime Crashes
**Debug Steps**:
1. Check Xcode console for crash logs
2. Verify all JUCE modules are properly linked
3. Test with minimal UI to isolate issues
4. Use Xcode's debugger for detailed analysis

## Distribution

### Development Distribution
- **TestFlight**: Up to 10,000 external testers
- **Ad Hoc**: Up to 100 devices registered in your developer account
- **Development**: Devices registered for development only

### App Store Distribution
1. **App Store Connect** setup required
2. **App Review** process (typically 1-7 days)
3. **Metadata** and screenshots required
4. **AUv3 plugins** can be distributed through App Store

### Enterprise Distribution
- **Apple Business Manager** required
- **Enterprise Certificate** ($299/year)
- **Internal distribution** only

## Bundle ID Best Practices

### Recommended Pattern
```
Company: Apple Developer Team Name
Base ID: com.yourcompany.pluginname
App: com.yourcompany.pluginname
AUv3: com.yourcompany.pluginname.AUv3
```

### Examples
```
✅ Good:
com.company.rhythmpattern
com.company.rhythmpattern.AUv3

❌ Bad:
com.company.rhythm_pattern     (underscore)
com.company.rhythmpatternAUv3  (missing dot)
com.company.plugin             (too generic)
```

## Additional Resources

### Apple Documentation
- [Audio Unit Programming Guide](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/)
- [App Distribution Guide](https://developer.apple.com/documentation/xcode/distributing-your-app-for-beta-testing-and-releases)

### JUCE Resources
- [JUCE iOS Deployment](https://docs.juce.com/master/tutorial_ios_projucer_and_cmake.html)
- [JUCE Forum](https://forum.juce.com/) - Community support

### Testing Tools
- **Console.app** (macOS) - View device logs
- **Instruments** - Performance profiling
- **Audio MIDI Setup** - MIDI routing verification

---

**Remember**: Always test on multiple iPad models and iOS versions before final release. The iPad Pro, iPad Air, and standard iPad may behave differently due to processing power and screen size variations.