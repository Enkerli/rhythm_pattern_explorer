# Enhanced Pattern Database with Perfect Balance Explorer

A comprehensive web application for analyzing and exploring rhythmic patterns using mathematical principles including perfect balance analysis, center of gravity calculations, and pattern combinations.

## Features

- **Universal Pattern Input**: Support for multiple pattern formats (polygons, Euclidean rhythms, binary, hex, octal, decimal)
- **Pattern Rotation**: Rotate any pattern using `@` notation (e.g., `0x92@3`)
- **Pattern Combinations**: Combine multiple patterns using logical operations with automatic LCM calculation
- **Perfect Balance Analysis**: Milne's perfect balance theory implementation with geometric visualization
- **Center of Gravity**: Mathematical analysis of pattern balance using complex number representation
- **Pattern Database**: Store, search, filter, and manage pattern collections with localStorage persistence
- **Systematic Exploration**: Automated discovery of perfect balance patterns through systematic exploration
- **Repetition Analysis**: Detect repeating patterns and calculate compression ratios
- **Structure Analysis**: Calculate density, silence gaps, and onset intervals
- **Non-intrusive Notifications**: Smart notification system that doesn't interrupt workflow

## Quick Start

1. Open `app/index.html` in a modern web browser
2. Enter a pattern in the Universal Pattern Input field (e.g., `P(3,1)+P(5,0)`)
3. Press **Enter** to parse and analyze, or **Ctrl/Cmd+Enter** to parse and automatically add to database
4. Use "Add to Database" button to save any current pattern (especially useful for random patterns and sequencer modifications)
5. Click "Parse & Analyze" button for analysis only

## Supported Pattern Formats

### Polygon Patterns
- `P(3,1)` - Triangle with offset 1
- `P(5,0)` - Pentagon at position 0
- `P(4,2,3)` - Square with offset 2 and expansion 3

### Euclidean Rhythms
- `E(5,8,0)` - 5 beats in 8 steps with no offset (Quintillo)
- `E(3,8,2)` - 3 beats in 8 steps with offset 2 (Tresillo)
- `comp E(3,8)` - Euclidean complement (E(5,8) - fills remaining positions)

### Binary/Hex/Octal/Decimal
- `b101010` - Binary pattern
- `0x92` - Hexadecimal pattern
- `0o452` - Octal pattern (useful for ternary rhythms)
- `146` - Decimal pattern
- `146:8` - Decimal with explicit step count
- `0x92:8` - Hex with explicit step count
- `0o452:9` - Octal with explicit step count

### Onset Arrays
- `[0,3,6]` - Onset positions (auto-determined step count)
- `[0,3,6]:8` - Onset positions with explicit step count
- `[0,2,4,6,7]` - Multiple onset positions

### Pattern Rotation
- `0x92@3` - Rotate pattern by 3 steps
- `P(5,0)@2` - Rotate pentagon by 2 steps  
- `b101010@-1` - Rotate binary pattern by -1 steps (counter-clockwise)

### Pattern Combinations
- `P(3,1)+P(5,0)` - Addition (logical OR)
- `P(3,0)+P(5,1)-P(2,0)` - Addition and subtraction
- `P(3,1)+P(5,0)+P(7,2)` - Multiple pattern combination

## File Structure

```
app/
├── index.html                    # Main application HTML
├── app-styles.css               # Application styles and UI components
├── math-core.js                 # Mathematical utilities and core functions
├── app-config.js                # Application configuration and constants
├── pattern-generators.js        # Pattern generation algorithms (Polygon, Euclidean)
├── pattern-analysis.js          # Analysis tools (balance, CoG, repetition)
├── pattern-processing.js        # Pattern parsing and conversion utilities
├── pattern-exploration.js       # Systematic pattern exploration and discovery
├── pattern-database.js          # Database management with localStorage
├── sequencer-audio.js           # Audio synthesis and playback
├── sequencer-visual.js          # Visual sequencer with CoG display
├── sequencer-controller.js      # Sequencer integration and controls
├── ui-components.js             # User interface components and utilities
└── app-controller-browser.js    # Main application controller and orchestrator
```

## Browser Compatibility

- Modern browsers with ES6 support
- localStorage required for pattern database persistence
- No external dependencies or build process required

## Usage Examples

### Basic Analysis
1. Enter `P(5,0)` to analyze a pentagon pattern
2. View perfect balance score, center of gravity, and structure analysis

### Euclidean Complements
1. Enter `E(3,8)` to see the classic Tresillo pattern
2. Enter `comp E(3,8)` to see its Euclidean complement (E(5,8))
3. Notice how they fill all positions without overlap

### Pattern Combinations
1. Enter `P(3,1)+P(5,0)` to combine triangle and pentagon
2. See LCM calculation and combined pattern analysis
3. Save to database for later reference

### Database Operations
1. Use search bar to find patterns by name or characteristics
2. Filter by type using dropdown (perfect balance, polygons, euclidean, combined, etc.)
3. Filter by step count range using min/max step inputs
4. Export/import pattern collections as JSON
5. Toggle between date-based and step-count sorting

### Exploration
1. Set exploration parameters using comma-separated format: `min sides, max sides, max patterns` (e.g., `3,7,3`)
2. Click "Find Perfect Balance" to discover patterns with perfect mathematical balance
3. Results automatically added to database with duplicate detection
4. Generate comprehensive reports of perfect balance patterns

## Mathematical Background

- **Perfect Balance**: Based on Milne's theory using complex number representation
- **Center of Gravity**: Geometric analysis of pattern distribution on unit circle
- **LCM Calculations**: Automatic least common multiple for pattern combinations
- **Euclidean Rhythms**: Implementation of Bjorklund's algorithm

## Development

The application uses a modular architecture with browser-compatible ES5/ES6 JavaScript. All modules are loaded sequentially to ensure proper dependency resolution. Features include:

- **Sequencer Integration**: Real-time audio playback with visual feedback
- **Interactive Visualization**: Circular pattern display with center of gravity indicators  
- **Advanced Pattern Detection**: Automatic recognition of Euclidean rhythms and repetitive patterns
- **Keyboard Shortcuts**: Enter to parse, Ctrl/Cmd+Enter to parse and save
- **Smart Notifications**: Non-intrusive feedback system
- **Comprehensive Analysis**: Perfect balance, center of gravity, structure analysis

## License

Open source - feel free to modify and distribute.