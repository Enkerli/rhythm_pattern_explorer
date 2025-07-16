# Claude Code Memory

## Project Configuration
- **Plugin Installation**: Install to user library (`~/Library/Audio/Plug-Ins/`) NOT system library (`/Library/Audio/Plug-Ins/`)
- **NO STANDALONE APP**: User explicitly does NOT want a standalone application - plugins only (AU/VST3)
  - Remove any `.app` files that get built accidentally
  - Focus on plugin formats only: AU Component and VST3
- **Licensing**: CC0 1.0 Universal (public domain)
- **JUCE Version**: 8.0.8 with WebBrowserComponent support

## Plugin Structure
- **AU Component**: `~/Library/Audio/Plug-Ins/Components/Rhythm Pattern Explorer.component`
- **VST3 Plugin**: `~/Library/Audio/Plug-Ins/VST3/Rhythm Pattern Explorer.vst3`
- **Build Commands**: Use Xcode schemes for clean builds when needed

## WebView Documentation
- Integrated using JUCE 8's WebBrowserComponent
- Toggle button switches between pattern UI and documentation
- HTML documentation includes comprehensive UPI syntax guide
- File-based delivery (not data URLs) for proper rendering

## Recent Issues Resolved  

### CRITICAL: Visual Accent Marker Cycling Fix (July 2025)
**Problem**: Visual accent markers weren't updating at cycle boundaries for polyrhythmic patterns like `{10}E(3,8)`, causing them to stay fixed instead of cycling as expected.

**Root Cause**: Accent offset calculation used `targetStep / pattern.size()` which always gave 0 at step 0, instead of tracking the actual cycle number from transport position.

**Solution**: Fixed accent offset calculation using absolute transport position:
```cpp
// BROKEN - only knew current step within cycle:
int completedCycles = targetStep / pattern.size(); // Always 0 at step 0

// FIXED - uses absolute transport position:
int absoluteStep = static_cast<int>(sampleStepsFromStart);
int currentCycle = absoluteStep / pattern.size();
int cycleStartAccentPosition = (currentCycle * onsetsPerCycle) % currentAccentPattern.size();
```

**Result**: `{10}E(3,8)` now properly cycles accent markers between:
- Cycle 0: accents at steps 0(red), 3(green), 6(red)
- Cycle 1: accents at steps 0(green), 3(red), 6(green)
- Creates proper polyrhythmic visual feedback

**CRITICAL PROTECTION**: See `Plugin/Tests/ACCENT_SYSTEM_PRINCIPLES.md` for architecture that MUST be preserved.

### CRITICAL: MIDI Trigger Logic Fix (July 2025)
**Problem**: Patterns with both progressive transformations and scenes (like `{1000000}E(1,16)E>16|1010101010101010`) only triggered progressive transformations via MIDI input, never advancing to scenes, unlike Enter key behavior.

**Root Cause**: MIDI trigger logic used `if/else if` statements, so when progressive transformations were detected, scene logic was never reached.

**Solution**: Changed conditional structure to handle both features when present together:
```cpp
// Before (PluginProcessor.cpp lines 1106-1137) - BROKEN:
if (hasProgressiveTransformation) {
    // Only triggers progressive, never reaches scene logic
} else if (hasScenes) {
    // Never reached when both are present
}

// After - FIXED:
bool triggerNeeded = false;
if (hasProgressiveTransformation) {
    parseAndApplyUPI(currentUPIInput, true);
    triggerNeeded = true;
}
// Handle scenes separately - can occur together with progressive transformations
if (hasScenes) {
    advanceScene();
    applyCurrentScenePattern(); 
    triggerNeeded = true;
}
```

**Result**: MIDI triggers now behave consistently with Enter key for all pattern combinations.

### CRITICAL: Accent Pattern Synchronization (July 2025)
**SOLUTION ARCHITECTURE** - Protect this implementation at all costs:

**Problem**: Accent markers were moving every step during playback (swirling effect) instead of staying stable and updating only at cycle boundaries.

**Root Cause**: UI accent display was using real-time `globalAccentPosition` which changes every step during playback.

**Solution**: Implemented dual-offset system:
- `globalAccentPosition`: Real-time accent tracking for MIDI output (advances every onset)
- `uiAccentOffset`: Stable accent offset for UI display (updates only at cycle boundaries and manual triggers)

**Key Implementation Details** (`PluginProcessor.cpp`):
```cpp
// In header - PluginProcessor.h line 230
int uiAccentOffset = 0;  // Stable accent offset for UI display

// Cycle boundary update - lines 387-393  
if (newStep == 0 && hasAccentPattern) {
    uiAccentOffset = globalAccentPosition % currentAccentPattern.size();
    patternChanged.store(true); // Notify UI to update
}

// UI display function - getCurrentAccentMap() uses uiAccentOffset
int accentStep = (uiAccentOffset + accentCounter) % accentLen;
accentMap[i] = currentAccentPattern[accentStep];

// Manual trigger resets - both variables reset to 0
globalAccentPosition = 0;
uiAccentOffset = 0;
```

**Expected Behavior**: 
- `{100}E(2,7)`: Accent at step 0 first cycle, step 3 second cycle  
- Stable display during playback, updates only at cycle boundaries
- Manual triggers (Enter, Tick, MIDI) reset both accent position and step indicator

### Manual Trigger Behavior (July 2025)
**Step Indicator Reset**: Added `currentStep.store(0)` to ALL manual trigger paths:
- Enter key: `setUPIInput()` 
- Tick parameter: `setUPIInput()` called from tick handler
- MIDI input: All trigger branches include step reset
- Scene cycling: `applyCurrentScenePattern()` includes reset

**Scene/Progressive Control**: Disabled auto-advancing at cycle boundaries
- Scenes and progressive transformations advance ONLY on manual triggers
- Cycle boundary code only updates UI accent offset, no pattern advancement

**Consistent Reset Pattern**: All manual triggers call:
```cpp
parseAndApplyUPI(pattern, true);  // resetAccentPosition = true
currentStep.store(0);             // Reset step indicator  
patternChanged.store(true);       // Notify UI updates
```

### Legacy Accent Pattern Support 
- **Accent Pattern Support**: Implemented suprasegmental accent layer system using curly bracket notation
  - `{100}E(3,8)` or `E(3,8){100}` adds accents on first onset of tresillo
  - `{10010}E(5,8)` creates accents on onsets 1 and 4 of quintillo  
  - Supports all pattern notations: `{E(2,5)}`, `{P(3,0)}`, `{101}`, etc.
  - **Polyrhythmic Cycling**: When accent and rhythm patterns have different lengths, accents cycle through creating polyrhythmic feel
  - **MIDI Output**: Accented notes have +5 semitones pitch and max velocity (1.0 vs 0.8)
  - **Visual Indication**: Accented onsets display in red vs green for regular onsets
- **B(n,s), W(n,s), D(n,s) Pattern Notations**: Restored direct pattern generation syntax
  - `B(3,7)` generates Barlow indispensability pattern with 3 onsets on 7 steps
  - `W(5,17)` generates Wolrab (anti-Barlow) pattern with 5 onsets on 17 steps  
  - `D(17,29)` generates Dilcue (anti-Euclidean) pattern with 17 onsets on 29 steps
  - Complements existing `E(n,s)` Euclidean notation for complete pattern language
  - Case-insensitive: `b(3,7)`, `w(5,17)`, `d(17,29)` also work
- **Progressive Transformation Looping**: Fixed transformations getting "stuck" at target onsets
  - Progressive transformations now loop back to base pattern when target is reached
  - `E(1,8)>8` now cycles: base → target → base → target (instead of staying at target)
  - Essential for live performance and scene-based patterns where continuous evolution is needed
  - Works with all transformation types: Barlow (B), Wolrab (W), Euclidean (E), Dilcue (D)
- **Barlow Transformation Regression**: Fixed sequential filling in ALL prime patterns (5, 7, 11, 13, 17, etc.) 
  - Implemented universal Evanstein SCAMP algorithm for proper hierarchical indispensability
  - `1000000B>7` now produces `1000001 → 1001001 → 1001101` (hierarchical) instead of `1000001 → 1100001 → 1110001` (sequential)
  - `1000000:17B>17` now produces proper fractional-based hierarchy, not sequential filling
  - Works for ALL step counts using musical fraction alignment and metric strength calculation
- State persistence problems with MIDI note reset - reverted to stable v0.02a approach
- WebView implementation with proper toggle and resize functionality
- Plugin installation to correct user library directories
- Releases page link added to webapp header
- **Parameter Interface Cleanup**: Removed irrelevant host parameters (BPM, Pattern Type, Onsets, Steps, Playing), kept essential ones (Use Host Transport, MIDI Note), added Trigger parameter for host automation
- **Hex Notation Inversion Fix**: Implemented strict Left-to-Right hex notation where `1000` = `0x1`, `0100` = `0x2`, `0010` = `0x4`, `0001` = `0x8`. Fixed in `PatternEngine.cpp` using LSB-first conversion for hex/octal/decimal display functions
- **Debug Logging Centralization**: Implemented centralized debug logging utility with category-based routing and `#ifdef DEBUG` guards for performance

## Build Process
```bash
# Clean rebuild plugin formats only (NO STANDALONE)
xcodebuild -scheme "Rhythm Pattern Explorer - AU" -configuration Release clean build
xcodebuild -scheme "Rhythm Pattern Explorer - VST3" -configuration Release clean build

# Install to user library (correct paths)
cp -R "Plugin/Builds/MacOSX/build/Release/Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/
cp -R "Plugin/Builds/MacOSX/build/Release/Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/

# Clean up any accidentally built standalone apps
rm -rf "Plugin/Builds/MacOSX/build/Release/Rhythm Pattern Explorer.app"
```

## Project Structure (Clean)
```
rhythm_pattern_explorer/
├── LICENSE                 # CC0 1.0 Universal
├── README.md              # Main project documentation
├── CHANGELOG.md           # Version history
├── INSTALLATION.md        # Installation instructions  
├── RELEASE_NOTES.md       # Current release notes
├── CLAUDE.md              # This file - project memory
├── Plugin/                # JUCE plugin source and builds
│   ├── Source/           # C++ source files
│   ├── JuceLibraryCode/  # JUCE generated files
│   ├── Builds/MacOSX/    # Xcode project and builds
│   │   ├── build/Release/ # Built AU component
│   │   └── Rhythm Pattern Explorer.vst3/ # Built VST3
│   ├── Archive/          # Historical versions and tests
│   ├── Documentation/    # Plugin-specific docs
│   └── Tests/           # Plugin test suites
├── WebApp/               # Browser-based pattern explorer
│   ├── app/             # Main web application files
│   └── [support files] # MIDI bridges, documentation
├── app -> WebApp/app    # Symlink for convenience
└── releases/            # Official release packages
    └── v0.02a/         # Versioned releases
```

## Current Status  
- **v0.03e with STABLE accent pattern cycling at boundaries** 
- **CRITICAL BUG FIXES COMPLETED** (July 2025):
  - ✅ Accent markers stable during playback (no swirling)
  - ✅ Accent patterns cycle properly at pattern boundaries (polyrhythmic visual feedback)  
  - ✅ Step indicator resets on all manual triggers
  - ✅ Manual-only scene/progressive advancement
  - ✅ Consistent reset behavior across all trigger paths
  - ✅ Perfect tick 1 timing with transport synchronization
- Both AU and VST3 plugins properly installed and up-to-date
- **Complete Pattern Language**: E(n,s), P(n,s), R(n,s), B(n,s), W(n,s), D(n,s) all working
- **Accent Pattern System**: Suprasegmental accent layer with curly bracket notation `{accent}pattern`
- Streamlined parameter interface: only 3 essential parameters exposed to host
- Correct Left-to-Right hex/decimal/octal notation implemented
- **Fixed progressive transformations**: Now loop continuously instead of getting stuck at target
- **Fixed Barlow transformations**: Universal hierarchical indispensability for ALL step counts including primes
- WebApp includes releases page link in header
- Project folder cleaned and organized

## Pattern Examples
```
# Basic patterns
E(3,8)           # Euclidean tresillo: 10010010
B(3,8)           # Barlow: 10001010  
W(3,8)           # Wolrab: 01100100
D(3,8)           # Dilcue: 01101001

# Accent patterns  
{100}E(3,8)      # Accented tresillo: first onset accented
{10010}E(5,8)    # Quintillo with accents on onsets 1 and 4
{E(2,5)}E(3,8)   # Complex polyrhythmic accents using Euclidean accent pattern
{10}E(4,8)       # Alternating accents creating polyrhythmic feel

# Progressive with accents
{101}E(1,8)B>8   # Accented progressive Barlow transformation
{P(3,0)}E(3,8)|B(5,13)  # Scene cycling with accent patterns
```

## Code Quality Improvements Completed
- **Quick Win #1**: Centralized debug logging with category-based routing
- **Quick Win #3**: Removed production debug file I/O operations
- **Hex Notation Fix**: Complete LSB-first conversion for all numeric displays
- **Pattern Parsing Consolidation**: Implemented generic numeric pattern handler that consolidates hex/decimal/octal parsing logic, reducing code repetition by ~80 lines and improving maintainability
- **Pattern Function Centralization**: Moved `bjorklundAlgorithm` and `rotatePattern` from PatternEngine to UPIParser, eliminating code duplication and creating single source of truth for pattern manipulation utilities
- **Pattern Recognition Refactor**: Implemented table-driven approach for pattern recognition, reducing boilerplate code and making it easier to add new pattern types
- **Progressive State Cleanup**: Added LRU-based cleanup mechanism to prevent unbounded growth of progressive transformation state map, with configurable size limit (100 patterns) and access tracking