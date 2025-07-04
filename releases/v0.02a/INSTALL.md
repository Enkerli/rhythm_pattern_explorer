# Installation Guide - Rhythm Pattern Explorer v0.02a

## Quick Installation

### For VST3 Users (Recommended)
```bash
# Copy to user plugins folder (no admin required)
cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/

# OR copy to system folder (requires admin)
sudo cp -R "Rhythm Pattern Explorer.vst3" /Library/Audio/Plug-Ins/VST3/
```

### For AU Users (Logic Pro, GarageBand)
```bash
# Copy to user plugins folder (no admin required)
cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/

# OR copy to system folder (requires admin)
sudo cp -R "Rhythm Pattern Explorer.component" /Library/Audio/Plug-Ins/Components/
```

## Step-by-Step Installation

### 1. Download and Extract
- Download the release package
- Extract to a temporary folder
- You'll see two plugin files:
  - `Rhythm Pattern Explorer.vst3` (VST3 format)
  - `Rhythm Pattern Explorer.component` (AU format)

### 2. Choose Your Plugin Format

**VST3 (Universal - works in most DAWs)**
- More compatible across different DAWs
- Modern plugin standard
- Recommended for most users

**AU (Audio Units - Apple's format)**
- Required for Logic Pro and GarageBand
- Native macOS format
- Tight integration with Apple DAWs

### 3. Copy to Plugin Folders

**Option A: User Installation (Recommended)**
- No administrator password required
- Only available to your user account
- Safer and easier to manage

**Option B: System Installation**
- Requires administrator password
- Available to all users on the system
- Use if multiple users need the plugin

### 4. Plugin Folder Locations

#### VST3 Folders:
- **User**: `~/Library/Audio/Plug-Ins/VST3/`
- **System**: `/Library/Audio/Plug-Ins/VST3/`

#### AU Folders:
- **User**: `~/Library/Audio/Plug-Ins/Components/`
- **System**: `/Library/Audio/Plug-Ins/Components/`

### 5. Verify Installation

1. **Open Finder**
2. **Press Cmd+Shift+G** (Go to Folder)
3. **Type the plugin folder path** (e.g., `~/Library/Audio/Plug-Ins/VST3/`)
4. **Verify the plugin file is there**

## DAW Setup

### Logic Pro
1. **Open Logic Pro**
2. **Go to Logic Pro > Preferences > Audio**
3. **Click "Reset & Rescan Selection"** (if needed)
4. **Create MIDI track**
5. **Add plugin as MIDI effect**: Insert → MIDI Effects → Rhythm Pattern Explorer

### Ableton Live
1. **Open Ableton Live**
2. **Go to Preferences > Plug-Ins**
3. **Click "Rescan"** (if needed)
4. **Create MIDI track**
5. **Drag plugin from Browser** to MIDI track

### Other DAWs
1. **Rescan plugins** in your DAW
2. **Look for "Rhythm Pattern Explorer"** in MIDI Effects
3. **Load as MIDI effect** on any track

## Troubleshooting

### Plugin Not Appearing
```bash
# Check if file exists
ls -la ~/Library/Audio/Plug-Ins/VST3/
ls -la ~/Library/Audio/Plug-Ins/Components/

# Verify permissions
ls -la ~/Library/Audio/Plug-Ins/VST3/"Rhythm Pattern Explorer.vst3"
```

### Permission Issues
```bash
# Fix permissions if needed
chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/"Rhythm Pattern Explorer.vst3"
chmod -R 755 ~/Library/Audio/Plug-Ins/Components/"Rhythm Pattern Explorer.component"
```

### Force DAW Rescan
- **Logic Pro**: Logic Pro > Preferences > Audio > Reset & Rescan Selection
- **Ableton Live**: Preferences > Plug-Ins > Rescan
- **Cubase**: Studio > VST Plug-in Manager > Update
- **Pro Tools**: Setup > Preferences > Processing > Plug-in Search

### Clean Reinstall
```bash
# Remove old versions
rm -rf ~/Library/Audio/Plug-Ins/VST3/"Rhythm Pattern Explorer.vst3"
rm -rf ~/Library/Audio/Plug-Ins/Components/"Rhythm Pattern Explorer.component"

# Clear plugin cache (varies by DAW)
# Then reinstall following steps above
```

## Verification

### Test Pattern
1. **Load plugin** on MIDI track
2. **Enter pattern**: `E(3,8)`
3. **Click Parse**
4. **Play track** - you should see/hear MIDI pattern

### Expected Behavior
- **Plugin loads** without errors
- **Pattern input field** visible
- **Parse button** functional
- **MIDI output** generates rhythmic patterns
- **Version display** shows "v0.02a"

## Uninstallation

### Remove Plugin Files
```bash
# Remove VST3
rm -rf ~/Library/Audio/Plug-Ins/VST3/"Rhythm Pattern Explorer.vst3"

# Remove AU
rm -rf ~/Library/Audio/Plug-Ins/Components/"Rhythm Pattern Explorer.component"
```

### Clean Plugin Cache
Rescan plugins in your DAW to remove from plugin lists.

## Support

If you encounter issues:
1. **Check system requirements** (macOS 10.13+)
2. **Verify DAW compatibility** (VST3 or AU support)
3. **Try both plugin formats** (VST3 and AU)
4. **Check DAW documentation** for plugin installation
5. **Restart DAW** after installation

---

**Installation Complete!** Start creating rhythmic patterns with mathematical precision.