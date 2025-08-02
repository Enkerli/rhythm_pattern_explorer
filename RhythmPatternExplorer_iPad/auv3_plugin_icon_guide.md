# AUv3 Plugin Icon Setup Guide

## Two Types of Icons for AUv3 Plugins

### 1. App Icon (✅ Complete)
- **Purpose**: iOS Springboard, App Store
- **Location**: `Images.xcassets/AppIcon.appiconset/`
- **Sizes**: 20px to 1024px (18 total sizes)
- **Status**: ✅ Generated with donut icon

### 2. Plugin Icon (❌ Missing)
- **Purpose**: DAW hosts (GarageBand, Logic, AUM, etc.)
- **Location**: Embedded as resource in the plugin binary
- **Sizes**: Typically 32x32, 64x64, 128x128
- **Format**: PNG or embedded binary data

## AUv3 Plugin Icon Implementation Options

### Option A: Embedded Binary Resource (Recommended)
Add icon data directly to the plugin code:

```cpp
// In PluginProcessor.cpp or PluginEditor.cpp
const char* pluginIconData = 
    "iVBORw0KGgoAAAANSUhEUgAAAA..." // Base64 encoded PNG data
    
juce::Image getPluginIcon() {
    return juce::ImageFileFormat::loadFrom(pluginIconData, sizeof(pluginIconData));
}
```

### Option B: Resource File (Alternative)
1. Add PNG files to JUCE project as binary resources
2. Access via `BinaryData::icon_32x32_png`

### Option C: Dynamic Generation (Current Approach)
Generate the icon programmatically using the same donut drawing code.

## DAW Host Icon Requirements

Different hosts have different requirements:
- **Logic Pro**: Uses 32x32 and 64x64
- **GarageBand**: Uses 32x32
- **AUM**: Uses 64x64 and 128x128
- **Cubasis**: Uses 128x128

## Implementation Status

**Current**: Only app icons (Springboard) are configured
**Needed**: Plugin icon for DAW hosts

## Next Steps

1. Generate 32x32, 64x64, 128x128 plugin icons
2. Embed as binary resources in JUCE project
3. Implement `getIcon()` method in AudioProcessor
4. Test in multiple DAW hosts