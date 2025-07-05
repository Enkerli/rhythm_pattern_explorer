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
- State persistence problems with MIDI note reset - reverted to stable v0.02a approach
- WebView implementation with proper toggle and resize functionality
- Plugin installation to correct user library directories
- Releases page link added to webapp header

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
- v0.02a4 with CC0 license and WebView documentation
- Both AU and VST3 plugins properly installed and up-to-date
- WebApp includes releases page link in header
- Project folder cleaned and organized