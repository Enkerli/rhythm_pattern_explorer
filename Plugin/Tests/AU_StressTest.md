# Audio Unit (AU) Stress Test Suite - Rhythm Pattern Explorer

## Test Environment Setup

### Requirements
- macOS with AU host (Logic Pro, GarageBand, MainStage, etc.)
- Rhythm Pattern Explorer AU plugin installed
- MIDI Environment or external MIDI monitor
- Activity Monitor for performance tracking

### Installation Verification
```bash
# Check AU installation
ls -la ~/Library/Audio/Plug-Ins/Components/ | grep "Rhythm"
# Should show: Rhythm Pattern Explorer.component

# Validate AU with auval
auval -v aufx Rpe1 RPE1
# Should return "PASS" for validation
```

## AU-Specific Test Categories

### 1. AU Validation Tests

#### Test 1.1: auval Validation
- **Objective**: Verify AU passes Apple's validation
- **Command**: `auval -v aufx Rpe1 RPE1`
- **Expected**: All tests pass, no warnings
- **Status**: ⏳ Pending

#### Test 1.2: Logic Pro Integration
- **Objective**: Test in Logic Pro environment
- **Steps**:
  1. Open Logic Pro
  2. Create Software Instrument track
  3. Insert Rhythm Pattern Explorer as MIDI effect
  4. Verify in MIDI Effects slot
- **Expected**: Plugin appears in MIDI Effects, loads correctly
- **Status**: ⏳ Pending

#### Test 1.3: GarageBand Compatibility
- **Objective**: Test in GarageBand (simplified AU host)
- **Steps**:
  1. Open GarageBand
  2. Create Software Instrument track
  3. Add Rhythm Pattern Explorer
  4. Test basic functionality
- **Expected**: Plugin works in simplified environment
- **Status**: ⏳ Pending

### 2. AU-Specific Features

#### Test 2.1: AU Parameter Automation
- **Objective**: Test Logic's automation system
- **Steps**:
  1. Load plugin in Logic Pro
  2. Open Automation menu
  3. Select plugin automation
  4. Create automation for BPM parameter
  5. Record automation data
- **Expected**: Parameters appear in automation menu, record correctly
- **Status**: ⏳ Pending

#### Test 2.2: AU State Saving
- **Objective**: Test project save/load with AU
- **Steps**:
  1. Set custom pattern E(5,12)
  2. Adjust BPM to 140
  3. Save Logic project
  4. Close and reopen project
  5. Verify plugin state restored
- **Expected**: All settings restored correctly
- **Status**: ⏳ Pending

#### Test 2.3: AU Preset System
- **Objective**: Test AU preset management
- **Steps**:
  1. Create custom pattern settings
  2. Save as AU preset from plugin menu
  3. Reset plugin to defaults
  4. Load saved preset
  5. Verify settings restored
- **Expected**: Preset system works correctly
- **Status**: ⏳ Pending

### 3. Logic Pro Specific Tests

#### Test 3.1: Score Editor Integration
- **Objective**: Test MIDI output in Logic's Score Editor
- **Steps**:
  1. Set pattern E(3,8)
  2. Record 2 bars of MIDI output
  3. Open Score Editor
  4. Verify note placement matches pattern
- **Expected**: Notes appear at correct positions in score
- **Status**: ⏳ Pending

#### Test 3.2: Piano Roll Accuracy
- **Objective**: Verify MIDI timing in Piano Roll
- **Steps**:
  1. Set BPM to 120
  2. Use pattern E(4,16) 
  3. Record 1 bar
  4. Check Piano Roll timing
- **Expected**: Notes align perfectly to 16th note grid
- **Status**: ⏳ Pending

#### Test 3.3: Flex Time Compatibility
- **Objective**: Test with Logic's Flex Time enabled
- **Steps**:
  1. Enable Flex Time on track
  2. Record pattern output
  3. Apply tempo changes via Flex Time
  4. Check plugin follows tempo
- **Expected**: Plugin adapts to Flex Time changes
- **Status**: ⏳ Pending

### 4. MainStage Live Performance Tests

#### Test 4.1: Concert Loading
- **Objective**: Test plugin in MainStage concert
- **Steps**:
  1. Create MainStage concert
  2. Add Software Instrument
  3. Insert Rhythm Pattern Explorer
  4. Save and load concert
- **Expected**: Fast loading, stable in live environment
- **Status**: ⏳ Pending

#### Test 4.2: Patch Changes
- **Objective**: Test switching between patches
- **Steps**:
  1. Create multiple patches with different patterns
  2. Switch between patches rapidly
  3. Monitor CPU and stability
- **Expected**: Smooth patch changes, no audio glitches
- **Status**: ⏳ Pending

#### Test 4.3: MIDI Controller Integration
- **Objective**: Test external MIDI control
- **Steps**:
  1. Assign MIDI CC to BPM parameter
  2. Use external controller to change BPM
  3. Test real-time parameter changes
- **Expected**: Responsive MIDI control, no latency issues
- **Status**: ⏳ Pending

### 5. Multi-Channel AU Tests

#### Test 5.1: Multi-Output Configuration
- **Objective**: Test if plugin can be configured for multi-output
- **Steps**:
  1. Check AU configuration in Logic
  2. Verify MIDI output routing options
  3. Test different channel assignments
- **Expected**: Proper MIDI channel handling
- **Status**: ⏳ Pending

#### Test 5.2: Sidechaining Support
- **Objective**: Verify no sidechain input issues
- **Steps**:
  1. Check for unwanted audio inputs
  2. Verify MIDI-only operation
  3. Test in sidechain scenarios
- **Expected**: No audio input conflicts
- **Status**: ⏳ Pending

### 6. AU Performance Benchmarks

#### Test 6.1: Host CPU Usage
- **Objective**: Measure AU-specific CPU overhead
- **Setup**: Logic Pro with 16 instances
- **Target**: <3% CPU per instance at 120 BPM
- **Status**: ⏳ Pending

#### Test 6.2: AU Memory Footprint
- **Objective**: Measure memory usage in AU host
- **Command**: 
```bash
sudo fs_usage -w -f filesys | grep "Rhythm Pattern Explorer"
vmmap $(pgrep Logic) | grep -i rhythm
```
- **Target**: <25MB per instance
- **Status**: ⏳ Pending

#### Test 6.3: Load Time Performance
- **Objective**: Measure AU loading speed
- **Test**: Time from insert to ready state
- **Target**: <3 seconds in Logic Pro
- **Status**: ⏳ Pending

### 7. AU Error Handling

#### Test 7.1: Host Disconnection
- **Objective**: Test behavior when host disconnects
- **Steps**:
  1. Load plugin in Logic
  2. Force quit Logic while plugin running
  3. Check for zombie processes
  4. Restart Logic and reload
- **Expected**: Clean shutdown, no zombie processes
- **Status**: ⏳ Pending

#### Test 7.2: AU Bypass Testing
- **Objective**: Test AU bypass functionality
- **Steps**:
  1. Enable pattern playback
  2. Bypass plugin using Logic's bypass
  3. Re-enable plugin
  4. Check state preservation
- **Expected**: Clean bypass, state preserved
- **Status**: ⏳ Pending

### 8. Compatibility Matrix

| DAW | Version | Load | Play | Automation | Presets | Notes |
|-----|---------|------|------|------------|---------|-------|
| Logic Pro | 11.x | ⏳ | ⏳ | ⏳ | ⏳ | Primary target |
| Logic Pro | 10.x | ⏳ | ⏳ | ⏳ | ⏳ | Legacy support |
| GarageBand | Latest | ⏳ | ⏳ | ⏳ | ⏳ | Consumer target |
| MainStage | 3.x | ⏳ | ⏳ | ⏳ | ⏳ | Live performance |
| Final Cut Pro | Latest | ⏳ | ⏳ | ⏳ | ⏳ | Audio post |

### 9. AU-Specific Stress Tests

#### Test 9.1: Parameter Flood
- **Objective**: Test rapid AU parameter changes
- **Method**: Automate all parameters simultaneously
- **Duration**: 10 minutes continuous
- **Expected**: Stable operation, no crashes
- **Status**: ⏳ Pending

#### Test 9.2: Project Complexity
- **Objective**: Test in complex Logic projects
- **Setup**: 
  - 64 tracks
  - Multiple AU instances
  - Heavy CPU load
  - Complex routing
- **Expected**: Plugin remains stable
- **Status**: ⏳ Pending

#### Test 9.3: Sample Rate Changes
- **Objective**: Test AU at different sample rates
- **Rates**: 44.1kHz, 48kHz, 88.2kHz, 96kHz
- **Expected**: Correct timing at all rates
- **Status**: ⏳ Pending

### 10. AU Debugging Tools

#### AU Lab Testing
```bash
# Use AU Lab for isolated testing
/Applications/AU\ Lab.app/Contents/MacOS/AU\ Lab
# Load Rhythm Pattern Explorer
# Test in controlled environment
```

#### Console Log Monitoring
```bash
# Monitor AU-specific logs
log stream --predicate 'category == "AudioUnit"' --level debug
```

#### Instruments Profiling
```bash
# Profile AU performance
instruments -t "Time Profiler" -D /tmp/au_profile.trace Logic\ Pro
```

### 11. AU Release Validation

#### Pre-Release AU Checklist
- [ ] auval validation passes
- [ ] Loads in Logic Pro without errors
- [ ] Works in GarageBand
- [ ] AU presets save/load correctly
- [ ] Automation works in Logic
- [ ] No Console errors during operation
- [ ] Stable under stress testing
- [ ] Compatible with latest macOS
- [ ] Signed and notarized correctly

#### AU Signing Verification
```bash
# Check AU code signing
codesign -dv --verbose=4 ~/Library/Audio/Plug-Ins/Components/Rhythm\ Pattern\ Explorer.component

# Verify notarization
spctl -a -v ~/Library/Audio/Plug-Ins/Components/Rhythm\ Pattern\ Explorer.component
```

### 12. AU Bug Report Template

#### AU Issue Report
- **Host**: [Logic Pro/GarageBand/MainStage version]
- **macOS**: [Version]
- **Plugin Version**: 2.8
- **Issue**: [Description]
- **Reproduction Steps**:
  1. [Step 1]
  2. [Step 2]
  3. [Step 3]
- **Expected Behavior**: [What should happen]
- **Actual Behavior**: [What actually happens]
- **Console Logs**: [Relevant log entries]
- **Crash Report**: [If applicable]

### 13. AU Performance Targets

| Metric | Target | Measurement Method |
|--------|--------|-------------------|
| CPU Usage | <3% per instance | Activity Monitor |
| Memory | <25MB per instance | Memory Monitor |
| Load Time | <3 seconds | Stopwatch |
| Latency | <5ms | Logic's latency display |
| auval | 100% pass | auval tool |