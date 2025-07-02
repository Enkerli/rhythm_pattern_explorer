# Rhythm Pattern Explorer - Web Application

This directory contains the **Web Application** implementation of Rhythm Pattern Explorer - the original browser-based version that serves as the reference implementation for all algorithms.

## Directory Structure

```
WebApp/
├── app/                        # Core web application
│   ├── index.html              # Main application entry point
│   ├── math-core.js            # Core mathematical algorithms
│   ├── pattern-generators.js   # Pattern generation algorithms  
│   ├── pattern-analysis.js     # Analysis and transformation algorithms
│   ├── app-controller-browser.js # Main application controller
│   ├── ui-components.js        # User interface components
│   ├── sequencer-*.js          # Audio/visual sequencer
│   └── *.js                    # Additional modules
├── serve-app.py                # Local development server
├── docs-maintenance.js         # Documentation management
├── minimal-midi-bridge.py      # MIDI bridge for external control
└── *.md                        # Shared documentation files
```

## Web Application Features

### Core Algorithms (Reference Implementation)
- **Euclidean Patterns**: Bjorklund algorithm implementation
- **Polygon Patterns**: Geometric rhythm generation
- **Barlow Transformations**: Indispensability theory algorithms
- **Progressive Transformations**: Step-by-step pattern evolution
- **UPI Parser**: Universal Pattern Input notation system

### Interactive Features
- **Real-time Pattern Visualization**: Circular and linear displays
- **Audio Playback**: Web Audio API integration
- **MIDI Output**: Web MIDI API support
- **Pattern Analysis**: Mathematical analysis tools
- **Documentation System**: Integrated help and examples

### Pattern Notation Examples
```
E(3,8)                    # Euclidean: 10010010
P(5,12,0)                 # Pentagon: 100100100100
E(1,8)E>8                 # Progressive: 1→11111111
E(8,8)B>1                 # Barlow dilution: 11111111→10000000
E(3,8)+P(5,12)           # Combination
E(3,8)*P(5,12)           # Multiplication
R(4,8,42)                # Random with seed
```

## Development Workflow

### Running the Web App
```bash
# Start local server
python3 serve-app.py

# Or use simple HTTP server
cd app
python3 -m http.server 8000

# Access at: http://localhost:8000
```

### MIDI Bridge (Optional)
```bash
# Enable external MIDI control
python3 minimal-midi-bridge.py
```

### Documentation Updates
```bash
# Update documentation
node docs-maintenance.js
```

## Algorithm Reference

The WebApp contains the **canonical implementations** of all algorithms used in the plugin. When developing the plugin, always reference these implementations for accuracy:

### Core Files
- **math-core.js**: Fundamental mathematical operations
- **pattern-generators.js**: Pattern generation algorithms
- **pattern-analysis.js**: Barlow indispensability, transformations
- **app-controller-browser.js**: Progressive transformation logic

### Key Functions
```javascript
// Euclidean patterns
EuclideanRhythm.generate(onsets, steps, rotation)

// Barlow transformations  
BarlowTransformer.transform(pattern, targetOnsets)

// Progressive stepping
ProgressiveTransformer.step(currentPattern, algorithm, direction)

// UPI parsing
UPIParser.parse(upiString)
```

## Browser Compatibility

### Supported Browsers
- **Chrome/Edge**: Full support (recommended)
- **Firefox**: Full support
- **Safari**: Full support (macOS/iOS)

### Required Web APIs
- **Web Audio API**: For audio playback
- **Web MIDI API**: For MIDI output (optional)
- **Canvas API**: For visualizations
- **ES6+ Support**: Modern JavaScript features

## Documentation Files

### Shared Documentation (in this directory)
- **README.md**: Main project documentation
- **PROGRESSIVE_TRANSFORMATION_REFERENCE.md**: Algorithm specifications
- **MIDI-SETUP-GUIDE.md**: MIDI configuration instructions
- **WORKING-MIDI-SOLUTIONS.md**: Troubleshooting guide

### Usage
These documentation files are shared between the WebApp and Plugin projects, providing unified documentation for both implementations.

## Testing

### Manual Testing
1. Open `app/index.html` in browser
2. Test pattern generation: `E(3,8)`, `P(5,12)`
3. Test progressive transformations: `E(1,8)E>8`
4. Verify audio playback and MIDI output
5. Test complex combinations and edge cases

### Algorithm Validation
The WebApp serves as the **reference implementation** for validating plugin algorithms. All pattern outputs should match between WebApp and Plugin versions.

## Deployment

### GitHub Pages
The WebApp can be deployed to GitHub Pages for public access:
```bash
# Push to gh-pages branch
git subtree push --prefix=WebApp/app origin gh-pages
```

### Local Hosting
```bash
# Production server
python3 serve-app.py --host 0.0.0.0 --port 8080
```

## Plugin Integration

### Algorithm Synchronization
The Plugin implementation should always match the WebApp algorithms:

1. **Reference**: WebApp implementations
2. **Port**: Convert JavaScript to C++
3. **Validate**: Ensure identical outputs
4. **Test**: Cross-verify pattern generation

### Shared Concepts
- **UPI Notation**: Identical syntax
- **Progressive Logic**: Same stepping algorithms  
- **Pattern Analysis**: Same mathematical calculations
- **Visual Layout**: Similar UI concepts

## Development History

### Original Implementation
The WebApp was the **original implementation** of Rhythm Pattern Explorer, developed over several years with extensive research into:
- Barlow's indispensability theory
- Euclidean rhythm algorithms
- Progressive transformation concepts
- Mathematical rhythm analysis

### Evolution to Plugin
The Plugin version was developed based on the mature WebApp algorithms, ensuring:
- **Algorithm Accuracy**: Direct translation from proven implementations
- **Feature Parity**: All core features available in both versions
- **Cross-validation**: WebApp serves as reference for plugin testing

## Contributing

### WebApp Development
- Maintain algorithm accuracy
- Keep documentation synchronized
- Test across multiple browsers
- Validate MIDI functionality

### Cross-Platform Considerations
- Changes to algorithms should be reflected in Plugin
- Documentation updates affect both projects
- Testing procedures should validate both implementations

## Related Projects

- **Plugin Implementation**: `../Plugin/` directory
- **Algorithm Documentation**: Shared in this directory
- **Test Procedures**: Reference implementations for plugin testing

## Live Demo

The WebApp provides a live demonstration of all Rhythm Pattern Explorer capabilities and serves as the primary reference for understanding the system's mathematical foundations.

**Access**: Open `app/index.html` in any modern web browser