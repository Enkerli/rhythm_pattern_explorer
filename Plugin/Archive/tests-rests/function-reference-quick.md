# Rhythm Pattern Explorer - Function Reference (Quick Guide)

## 🚀 Quick Function Index

### 📐 Mathematical Core Functions (math-core.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `MathUtils.gcd(a, b)` | Greatest Common Divisor | `gcd(12, 8) → 4` |
| `MathUtils.lcm(a, b)` | Least Common Multiple | `lcm(3, 5) → 15` |
| `MathUtils.lcmArray(nums)` | LCM of multiple numbers | `lcmArray([3,5,7]) → 105` |
| `MathUtils.isPrime(n)` | Test if number is prime | `isPrime(7) → true` |
| `MathUtils.areCoprimes(a, b)` | Test if numbers are coprime | `areCoprimes(9, 16) → true` |
| `MathUtils.getPrimeFactors(n)` | Get prime factorization | `getPrimeFactors(12) → [2,2,3]` |

### 🎵 Pattern Generation Functions (pattern-generators.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `RegularPolygonGenerator.generate(v, o, e)` | Create polygon patterns | `generate(3, 1, 2) → P(3,1,2)` |
| `RegularPolygonGenerator.getPolygonName(v)` | Get polygon name | `getPolygonName(5) → "Pentagon"` |
| `EuclideanGenerator.generate(b, s, o)` | Create Euclidean rhythms | `generate(3, 8) → Tresillo` |
| `EuclideanGenerator.generateComplement(b, s, o)` | Create Euclidean complement | `comp E(3,8) → E(5,8)` |

### 🔄 Pattern Conversion Functions (pattern-processing.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `PatternConverter.toBinary(steps, count)` | Convert to binary string | `[true,false,true] → "101"` |
| `PatternConverter.fromBinary(binary)` | Parse binary pattern | `"101" → [true,false,true]` |
| `PatternConverter.toHex(decimal)` | Convert to hexadecimal | `146 → "0x92"` |
| `PatternConverter.fromHex(hex)` | Parse hex pattern | `"0x92:8" → pattern` |
| `PatternConverter.toOnsetArray(steps, count)` | Convert to onset positions | `[true,false,true] → "[0,2]"` |

### 📊 Analysis Functions (pattern-analysis.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `PerfectBalanceAnalyzer.calculateBalance(steps, count)` | Analyze perfect balance | Returns balance metrics |
| `CenterOfGravityCalculator.calculateCenterOfGravity(steps)` | Calculate CoG | Returns {x, y} coordinates |
| `BarlowTransformer.transformPattern(pattern, target, opts)` | Transform using indispensability | Dilute/concentrate patterns |
| `BarlowTransformer.calculatePositionIndispensability(pos, len)` | Get position importance | `pos=0 → 1.0` (downbeat) |
| `SyncopationAnalyzer.calculateBarlowIndispensability(onsets, count)` | Measure syncopation | Returns syncopation score |

### 💾 Database Functions (pattern-database.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `PatternDatabase.add(pattern)` | Add pattern to database | Returns pattern ID or null |
| `PatternDatabase.remove(id)` | Remove pattern by ID | Returns success boolean |
| `PatternDatabase.search(query, filters)` | Search patterns | Returns matching patterns |
| `PatternDatabase.export()` | Export as JSON | Returns JSON string |
| `PatternDatabase.import(jsonData)` | Import from JSON | Returns import stats |
| `PatternDatabase.getStatistics()` | Get database stats | Returns counts by category |

### 🎼 Sequencer Functions (sequencer-*.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `SequencerAudio.initialize()` | Initialize Web Audio | Sets up audio context |
| `SequencerAudio.playSoundAtTime(time, freq, dur)` | Play timed sound | Precise audio scheduling |
| `SequencerVisual.drawPattern(pattern, canvas)` | Draw circular pattern | Visual representation |
| `SequencerController.startPlayback()` | Start pattern playback | Begin audio/visual sync |
| `SequencerController.updateTempo(bpm)` | Change playback speed | Update timing |

### 🎯 App Control Functions (app-controller-browser.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `EnhancedPatternApp.parsePattern()` | Parse universal input | Analyze user input |
| `EnhancedPatternApp.addPatternToDatabase()` | Save current pattern | Add to database |
| `EnhancedPatternApp.transformBarlowPattern()` | Apply Barlow transformation | Transform current pattern |
| `EnhancedPatternApp.progressiveBarlowTransform()` | Generate transformation sequence | Step-by-step changes |
| `EnhancedPatternApp.updatePatternList()` | Refresh pattern display | Update UI list |

## 🎨 UI Component Functions (ui-components.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `UIComponents.createPatternEntry(pattern)` | Create pattern list item | Generate HTML entry |
| `UIComponents.showModal(title, content)` | Display modal dialog | Show user feedback |
| `UIComponents.updateProgress(percent)` | Update progress bar | Show operation progress |
| `UIComponents.copyToClipboard(text)` | Copy to clipboard | Copy pattern data |

## 🔧 Configuration Functions (app-config.js)
| Function | Purpose | Example |
|----------|---------|---------|
| `AppConfig.getBalanceScoreColor(score)` | Get color for balance score | `"perfect" → "#4CAF50"` |
| `AppConfig.getPolygonName(vertices)` | Get polygon name | `5 → "Pentagon"` |
| `AppConfig.isFeatureEnabled(feature)` | Check feature flag | Returns boolean |
| `AppConfig.validatePattern(pattern)` | Validate pattern object | Returns validity |

---

## 🎵 Pattern Format Examples

### Input Formats
```javascript
// Euclidean rhythms
"E(3,8)"        // 3 beats in 8 steps
"E(5,8,2)"      // 5 beats in 8 steps, offset 2

// Polygon patterns  
"P(3,1)"        // Triangle with offset 1
"P(5,0,2)"      // Pentagon, no offset, 2x expansion

// Binary patterns
"b10010010"     // Binary notation
"0x92:8"        // Hexadecimal with step count
"[0,3,6]:8"     // Onset positions

// Combinations
"P(3,1)+P(5,0)" // Combined polygons
"~E(3,8)"       // Inverted pattern
"E(3,8)@2"      // Rotated pattern
```

### Barlow Transformations
```javascript
// Basic transformation
BarlowTransformer.transformPattern(pattern, 5)

// Wolrab mode (anti-metrical)
BarlowTransformer.transformPattern(pattern, 3, {wolrabMode: true})

// Progressive sequence
BarlowTransformer.progressiveTransform(pattern, 4, 8, "concentrate")
```

### Analysis Results
```javascript
// Perfect balance analysis
{
    magnitude: 0.125,
    isPerfectlyBalanced: false,
    balanceScore: "good",
    cogPosition: {x: 0.125, y: -0.33}
}

// Indispensability scores (16-step pattern)
// Position:        0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
// Indispensability: 1.0  0.06 0.25 0.06 0.5  0.06 0.25 0.06 0.75 0.06 0.25 0.06 0.5  0.06 0.25 0.75
```

---

## 🚀 Common Usage Patterns

### 1. Parse and Analyze Pattern
```javascript
const input = "E(3,8)";
const parsed = UnifiedPatternParser.parse(input);
const balance = PerfectBalanceAnalyzer.calculateBalance(parsed.steps, parsed.stepCount);
const cog = CenterOfGravityCalculator.calculateCenterOfGravity(parsed.steps);
```

### 2. Transform Pattern with Barlow
```javascript
const original = [true, false, true, false, true, false, false, false];
const concentrated = BarlowTransformer.transformPattern(original, 5);
const diluted = BarlowTransformer.transformPattern(original, 2);
```

### 3. Generate Progressive Sequence
```javascript
const sequence = BarlowTransformer.progressiveTransform(
    pattern, currentOnsets, targetOnsets, "concentrate"
);
// Returns array of intermediate patterns
```

### 4. Database Operations
```javascript
const db = new PatternDatabase();
const patternId = db.add(analyzedPattern);
const searchResults = db.search("tresillo", {category: "euclidean"});
const stats = db.getStatistics();
```

### 5. Sequencer Playback
```javascript
const sequencer = new SequencerController();
await sequencer.initialize();
sequencer.loadPattern(pattern);
sequencer.startPlayback();
sequencer.updateTempo(140);
```

---

## 🎯 Function Categories by Use Case

### For Pattern Generation
- `EuclideanGenerator.generate()` - Mathematical rhythm generation
- `RegularPolygonGenerator.generate()` - Geometric pattern creation
- `BarlowTransformer.transformPattern()` - Intelligent transformations

### For Pattern Analysis  
- `PerfectBalanceAnalyzer.calculateBalance()` - Balance metrics
- `SyncopationAnalyzer.calculateBarlowIndispensability()` - Syncopation analysis
- `CenterOfGravityCalculator.calculateCenterOfGravity()` - Geometric analysis

### For Data Management
- `PatternDatabase` methods - Storage and retrieval
- `PatternConverter` methods - Format conversions
- `UIComponents` methods - User interface

### For Audio/Visual
- `SequencerAudio` methods - Sound generation
- `SequencerVisual` methods - Pattern visualization
- `SequencerController` methods - Playback control

---

*This quick reference covers the most important functions in the Rhythm Pattern Explorer. For detailed documentation with parameters and examples, see the complete function documentation.*