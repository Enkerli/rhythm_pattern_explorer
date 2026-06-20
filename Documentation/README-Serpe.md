# Serpe - Sequence Editor, Rhythmic Pattern Explorer

Unified cross-platform rhythm pattern generation tool supporting macOS and iPadOS.

## Overview

Serpe consolidates the desktop and iPad implementations into a single codebase supporting:
- macOS: AU Component and VST3 Plugin formats
- iPadOS: AUv3 App Extension

## Pattern Types Supported

- Euclidean Rhythms: E(n,s)
- Barlow Indispensability: B(n,s) 
- Wolrab Anti-Barlow: W(n,s)
- Dilcue Anti-Euclidean: D(n,s)
- Progressive Transformations: E(3,8)>16
- Accent Patterns: {101}E(3,8)
- Scene Cycling: E(3,8)|B(5,13)

## Project Structure

- Source/Core/ - Shared algorithms
- Source/Platform/ - Platform-specific code
- Tests/ - Test suite
- Documentation/ - Project docs
- Builds/ - Build outputs

## License

CC0 1.0 Universal (Public Domain)