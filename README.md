# Rhythmic Pattern Explorer

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

1. Open `index.html` in a modern web browser
2. Enter a pattern in the Universal Pattern Input field (e.g., `P(3,1)+P(5,0)`)
3. Click "Parse & Analyze" to see comprehensive analysis
4. Use "Add to Database" to save patterns for later reference

## Supported Pattern Formats

### Polygon Patterns
- `P(3,1)` - Triangle with offset 1
- `P(5,0)` - Pentagon at position 0
- `P(4,2,3)` - Square with offset 2 and expansion 3

### Euclidean Rhythms
- `E(5,8,0)` - 5 beats in 8 steps with no offset (Quintillo)
- `E(3,8,2)` - 3 beats in 8 steps with offset 2 (Tresillo)

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
- `0x92@3` - Rotate Tresillo by 3 steps
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
├── app-styles.css               # Application styles
├── math-core.js                 # Mathematical utilities
├── app-config.js                # Application configuration
├── pattern-generators.js        # Pattern generation algorithms
├── pattern-analysis.js          # Analysis tools (balance, CoG, repetition)
├── pattern-processing.js        # Pattern parsing and conversion
├── pattern-exploration.js       # Systematic pattern exploration
├── pattern-database.js          # Database management
├── ui-components.js             # User interface components
├── app-controller-browser.js    # Main application controller
└── README.md                    # This file
```

## Browser Compatibility

- Modern browsers with ES6 support
- localStorage required for pattern database persistence
- No external dependencies or build process required

## Usage Examples

### Basic Analysis
1. Enter `P(5,0)` to analyze a pentagon pattern
2. View perfect balance score, center of gravity, and structure analysis

### Pattern Combinations
1. Enter `P(3,1)+P(5,0)` to combine triangle and pentagon
2. See LCM calculation and combined pattern analysis
3. Save to database for later reference

### Database Operations
1. Use search bar to find patterns by polygon type (e.g., "pentagon")
2. Filter by type using dropdown (polygons, euclidean, combined, etc.)
3. Export/import pattern collections as JSON

### Exploration
1. Set exploration parameters (min/max sides, combinations)
2. Click "Find Perfect Balance" to discover new patterns
3. Results automatically added to database

## Mathematical Background

- **Perfect Balance**: Based on Milne's theory using complex number representation
- **Center of Gravity**: Geometric analysis of pattern distribution on unit circle
- **LCM Calculations**: Automatic least common multiple for pattern combinations
- **Euclidean Rhythms**: Implementation of Bjorklund's algorithm

## Development

The application uses a modular architecture with browser-compatible ES5/ES6 JavaScript. All modules are loaded sequentially to ensure proper dependency resolution.

## License

Open source - feel free to modify and distribute.