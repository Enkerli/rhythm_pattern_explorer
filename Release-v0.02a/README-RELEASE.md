# Rhythm Pattern Explorer v0.02a - Release Package

## Quick Install

### For Music Producers (Recommended)

1. **Download** this release package
2. **Install plugins** by copying to your plugin directories:

```bash
# Audio Unit (AU) - for Logic Pro, GarageBand
cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/

# VST3 - for Bitwig, Cubase, Reaper, etc.
cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

3. **Restart your DAW** to scan the new plugins
4. **Load as MIDI Effect** (not instrument)

## What's Included

- `Rhythm Pattern Explorer.component` - Audio Unit format
- `Rhythm Pattern Explorer.vst3` - VST3 format  
- `INSTALLATION.md` - Detailed installation guide
- `RELEASE_NOTES.md` - Complete feature documentation
- `CHANGELOG.md` - Technical changes and fixes

## System Requirements

- **macOS**: 10.15 Catalina or later
- **Architecture**: Universal (Intel + Apple Silicon)
- **DAW**: Any AU/VST3 compatible host

## Quick Test

1. Load plugin as **MIDI effect** in your DAW
2. Enter `E(3,8)` in the UPI field 
3. Click **Parse** button
4. Start DAW playback
5. Verify pattern triggers and pie chart animation

## Major Fixes in v0.02a

✅ **Critical timing issues resolved** (20-666 BPM tested)  
✅ **BPM range expanded** from 60-180 to 60-1000 BPM  
✅ **Position sync interference eliminated**  
✅ **Clean interface** with UPI pattern input  
✅ **Validated in Bitwig Studio** 5.3.12 extensively  

## Support

- **Issues**: https://github.com/enkerli/rhythm_pattern_explorer/issues
- **Documentation**: See included INSTALLATION.md and RELEASE_NOTES.md

---

**Version**: v0.02a  
**Build Date**: July 2024  
**Formats**: AU, VST3 (no standalone app included)