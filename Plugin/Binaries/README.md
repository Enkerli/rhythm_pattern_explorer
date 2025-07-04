# Rhythm Pattern Explorer - Plugin Binaries

This directory contains the pre-built plugin binaries for macOS (Apple Silicon).

## Installation

Copy the appropriate files to your plugin directories:

- **Standalone**: `Rhythm Pattern Explorer.app` → `~/Library/Audio/Plug-Ins/Standalone/`
- **VST3**: `Rhythm Pattern Explorer.vst3` → `~/Library/Audio/Plug-Ins/VST3/`
- **AU**: `Rhythm Pattern Explorer.component` → `~/Library/Audio/Plug-Ins/Components/`

## Version

Current version: v0.03 (July 3, 2025)

## Features

- Mathematical rhythm pattern generation
- UPI (Universal Pattern Input) support
- Animated pie chart visualization
- Multiple notation formats (binary, hex, octal, decimal)
- Real-time MIDI output
- Pattern analysis and visualization

## Compatibility

- macOS 10.13+
- Apple Silicon (ARM64)
- JUCE 8.0.1

## Usage

1. Install the plugin in your preferred format
2. Load in your DAW or use the standalone application
3. Enter UPI patterns (e.g., `E(3,8)`, `b10110000`, `x9B`, `o233`)
4. Play patterns and view animated visualization