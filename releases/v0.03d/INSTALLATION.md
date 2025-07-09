# Installation Instructions

## macOS Installation

### Prerequisites
- macOS 10.13 or later
- A compatible DAW (Logic Pro, GarageBand, Bitwig Studio, Cubase, Reaper, Ableton Live, etc.)

### Installation Steps

1. **Download the plugins** from the GitHub release
2. **Install to correct locations**:
   - **AU Plugin**: Copy `Rhythm Pattern Explorer.component` to `~/Library/Audio/Plug-Ins/Components/`
   - **VST3 Plugin**: Copy `Rhythm Pattern Explorer.vst3` to `~/Library/Audio/Plug-Ins/VST3/`

### Command Line Installation
```bash
# Copy AU plugin
cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/

# Copy VST3 plugin  
cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

### DAW-Specific Setup

#### Logic Pro / GarageBand
1. Open Logic Pro or GarageBand
2. Create a new Software Instrument track
3. In the track inspector, click on the Instrument slot
4. Navigate to **AU Instruments** → **Rhythm Pattern Explorer**
5. Select **Rhythm Pattern Explorer**

#### Bitwig Studio
1. Open Bitwig Studio
2. Create a new Instrument track
3. Click the device slot and browse devices
4. Navigate to **Instruments** → **Rhythm Pattern Explorer**
5. Double-click to load

#### Cubase
1. Open Cubase
2. Create a new Instrument track
3. Click the instrument slot
4. Navigate to **VST Instruments** → **Rhythm Pattern Explorer**
5. Select and load

#### Reaper
1. Open Reaper
2. Create a new track
3. Click **FX** button on the track
4. Type "Rhythm" in the search box
5. Select **VST3: Rhythm Pattern Explorer**

#### Ableton Live
1. Open Ableton Live
2. Create a new MIDI track
3. In the Device Browser, navigate to **Instruments** → **VST3**
4. Drag **Rhythm Pattern Explorer** to the track

### Verification

1. **Check plugin loads**: The plugin should appear in your DAW's instrument list
2. **Open the interface**: You should see the UPI input field and documentation button
3. **Test basic function**: Try entering `E(3,8)` and clicking the trigger button
4. **Verify MIDI output**: Connect to a drum instrument and test pattern playback

### Troubleshooting

#### Plugin doesn't appear in DAW
- Verify you copied to the correct directory (`~/Library/Audio/Plug-Ins/`)
- Restart your DAW after installation
- Check DAW's plugin scanner/rescan function

#### Permission issues
```bash
# Fix permissions if needed
chmod -R 755 ~/Library/Audio/Plug-Ins/Components/Rhythm\ Pattern\ Explorer.component
chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/Rhythm\ Pattern\ Explorer.vst3
```

#### Plugin crashes or doesn't load
- Check you're using a compatible DAW version
- Verify you have macOS 10.13 or later
- Try both AU and VST3 formats to see which works better in your DAW

### Uninstallation

To remove the plugin:
```bash
rm -rf ~/Library/Audio/Plug-Ins/Components/Rhythm\ Pattern\ Explorer.component
rm -rf ~/Library/Audio/Plug-Ins/VST3/Rhythm\ Pattern\ Explorer.vst3
```

## Getting Started

After installation:
1. Load the plugin in your DAW
2. Click the **Documentation** button for complete usage instructions
3. Try basic patterns like `E(3,8)` (tresillo) or `E(5,8)` (quintillo)
4. Experiment with progressive transformations like `E(1,8)E>8`
5. Add accent patterns with `{100}E(3,8)`

## Support

For installation issues or questions:
- Check the main documentation via the plugin's Documentation button
- Visit: https://github.com/enkerli/rhythm_pattern_explorer
- Report issues: https://github.com/enkerli/rhythm_pattern_explorer/issues