# Rhythm Pattern Explorer v0.03c Installation Guide

## Quick Installation

1. **Download** the plugin package from the GitHub Releases page
2. **Extract** the downloaded archive
3. **Copy** plugins to your user library (NOT system library):
   - **AU Component**: Copy `Rhythm Pattern Explorer.component` to `~/Library/Audio/Plug-Ins/Components/`
   - **VST3 Plugin**: Copy `Rhythm Pattern Explorer.vst3` to `~/Library/Audio/Plug-Ins/VST3/`
4. **Restart** your DAW
5. **Load** the plugin as a MIDI effect in your DAW

## Detailed Installation Steps

### macOS Installation

#### Step 1: Download and Extract
- Download `RhythmPatternExplorer_v0.03c_macOS.zip` from GitHub Releases
- Double-click to extract the archive
- You should see two plugin files

#### Step 2: Install Plugins

**Method 1: Drag and Drop (Recommended)**
1. Open Finder
2. Press `Cmd+Shift+G` to open "Go to Folder"
3. Type `~/Library/Audio/Plug-Ins/Components/` and press Enter
4. Drag `Rhythm Pattern Explorer.component` to this folder
5. Repeat for VST3: Go to `~/Library/Audio/Plug-Ins/VST3/`
6. Drag `Rhythm Pattern Explorer.vst3` to this folder

**Method 2: Terminal Installation**
```bash
# Navigate to your downloaded files
cd ~/Downloads/RhythmPatternExplorer_v0.03c/

# Install AU Component
cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/

# Install VST3 Plugin
cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

#### Step 3: Verify Installation
1. Restart your DAW completely
2. Create a new project
3. Add a MIDI track
4. Look for "Rhythm Pattern Explorer" in your MIDI effects list
5. The plugin should appear under both AU and VST3 formats

## System Requirements

- **macOS**: 10.13 (High Sierra) or newer
- **Architecture**: Universal Binary (Intel + Apple Silicon)
- **DAW**: Any AU or VST3 compatible host
- **Disk Space**: ~7MB total

## Tested DAWs

- ✅ Logic Pro X/Pro
- ✅ Ableton Live
- ✅ Bitwig Studio
- ✅ Cubase/Nuendo
- ✅ Pro Tools (2020.1+)
- ✅ Reaper
- ✅ Studio One

## Troubleshooting

### Plugin Doesn't Appear
1. **Check Installation Path**: Ensure plugins are in user library, not system library
2. **Restart DAW**: Completely quit and restart your DAW
3. **Rescan Plugins**: Force a plugin rescan in your DAW preferences
4. **Check Architecture**: Ensure your DAW supports the plugin architecture

### Plugin Won't Load
1. **Check macOS Version**: Requires macOS 10.13+
2. **Security Settings**: On newer macOS, you may need to allow the plugin in Security & Privacy
3. **Path Spaces**: Ensure no special characters in installation path

### No MIDI Output
1. **MIDI Routing**: Ensure MIDI output is routed to a destination
2. **Plugin Type**: Load as MIDI effect, not audio effect
3. **Transport**: Check that host transport is playing or use internal clock

## Getting Started

1. **Load** the plugin as a MIDI effect
2. **Enter** a pattern: `E(3,8)` (tresillo)
3. **Add accents**: `{100}E(3,8)` (accent first onset)
4. **Route MIDI** to your drum machine/sampler
5. **Experiment** with polyrhythms: `{10}E(3,8)`

## Pattern Examples

```
# Basic patterns
E(3,8)           # Euclidean tresillo
{100}E(3,8)      # Accented tresillo
{10}E(4,8)       # Alternating accents

# Advanced patterns
{E(2,5)}E(3,8)   # Euclidean accents over Euclidean rhythm
{100100}B(3,7)   # 6-step accents over Barlow pattern
E(3,8)|P(5,0)    # Scene cycling between patterns
```

## Support

- **Documentation**: Included in plugin (WebView toggle)
- **Issues**: https://github.com/enkerli/rhythm_pattern_explorer/issues
- **License**: CC0 1.0 Universal (Public Domain)

---

**Installation Complete!** 🎵 Happy rhythm exploring!