# Rhythm Pattern Explorer - Performance Stress Testing Guide

## Version: v0.02b.240703.1730-PERF

This performance testing version includes comprehensive monitoring and logging to analyze plugin performance under extreme conditions.

## Performance Monitoring Features

### Audio Processing Monitoring
- **Buffer Size Tracking**: Min/max buffer sizes encountered
- **Processing Time**: Average processing time per audio callback
- **Call Frequency**: Number of processBlock calls per interval
- **Step Advancement**: Current step position and timing accuracy

### Animation Performance Monitoring  
- **Frame Rate**: UI animation FPS measurement
- **Pattern Complexity**: Step count and pattern size impact
- **Rendering Efficiency**: Repaint frequency and timing

### Timing Validation
- **BPM Range Testing**: Validates extreme BPM scenarios (1-1000+ BPM)
- **Samples Per Step**: Catches timing calculation errors
- **Transport Sync**: Host synchronization accuracy

## Stress Testing Scenarios

### 1. Extreme BPM Testing
**Test Range**: 1 BPM to 1000 BPM

```
Test Cases:
- Ultra-slow: 1-10 BPM (>22050 samples/step)
- Very slow: 10-30 BPM 
- Normal: 60-180 BPM
- Fast: 200-300 BPM
- Extreme: 400-600 BPM (<110 samples/step)
- Impossible: 700-1000+ BPM (<63 samples/step)
```

**Expected Behavior**:
- Plugin should handle all BPM ranges gracefully
- Warnings logged for extreme cases
- No crashes or audio dropouts

### 2. Buffer Size Stress Testing
**Test Range**: 32 to 8192 samples

```
Common Buffer Sizes:
- Minimum: 32 samples (0.7ms @ 44.1kHz)
- Low latency: 64-128 samples
- Standard: 256-512 samples  
- High latency: 1024-2048 samples
- Extreme: 4096-8192 samples
```

**Expected Behavior**:
- Consistent step timing across all buffer sizes
- Performance stats logged every 5 seconds
- No timing drift or step skipping

### 3. Pattern Complexity Testing
**Test Patterns**:

```
Simple: E(3,8) - 8 steps, 3 onsets
Medium: E(8,16) - 16 steps, 8 onsets  
Complex: E(16,32) - 32 steps, 16 onsets
Dense: E(31,32) - 32 steps, 31 onsets
Progressive: E(3,8)>E(5,13)>E(8,21) - Growing patterns
```

**Expected Behavior**:
- Animation FPS should remain >30 fps
- Pattern switching should be instant
- Memory usage should remain stable

### 4. Extended Playback Testing
**Duration**: 30+ minutes continuous playback

```
Test Scenarios:
- Continuous E(5,8) for 30 minutes
- Progressive pattern evolution
- Rapid pattern switching (every 10 seconds)
- Multiple plugin instances (2-4 instances)
```

**Expected Behavior**:
- No memory leaks
- Consistent timing accuracy
- Stable CPU usage
- No audio dropouts

## Performance Monitoring Console Output

The performance version logs detailed statistics:

```
PERFORMANCE STATS: Calls=430, MinBuffer=512, MaxBuffer=512, AvgProcessTime=0.023ms, CurrentStep=3, SamplesPerStep=2756

PERFORMANCE WARNING: Very fast timing - samplesPerStep=47 at BPM=600 (>450 BPM equivalent)

ANIMATION PERFORMANCE: FPS=59.8, PatternSize=8, CurrentStep=3, IsPlaying=YES
```

## How to Run Performance Tests

### Setup
1. Load plugin in your DAW (Logic Pro, Bitwig, etc.)
2. Open Console.app and filter for "PERFORMANCE"
3. Set up audio interface with various buffer sizes

### Test Execution
1. **BPM Stress Test**:
   - Start at 120 BPM with E(3,8)
   - Gradually increase to 600+ BPM
   - Monitor console for warnings
   - Check for audio dropouts

2. **Buffer Size Test**:
   - Set DAW buffer to 32 samples
   - Play E(5,8) for 5 minutes
   - Increase buffer size incrementally
   - Monitor performance stats

3. **Pattern Complexity Test**:
   - Test patterns: E(3,8) → E(8,16) → E(16,32) → E(31,32)
   - Monitor animation FPS
   - Check step advancement accuracy

4. **Extended Playback Test**:
   - Start 30-minute playback session
   - Monitor memory usage in Activity Monitor
   - Check for performance degradation over time

### Success Criteria
- ✅ No crashes under any BPM setting
- ✅ Processing time <1ms average
- ✅ Animation FPS >30 during playback
- ✅ Step timing accuracy ±1 sample
- ✅ Stable memory usage over time
- ✅ No audio dropouts at standard buffer sizes

## Common Issues to Watch For
- Step counter freezing at extreme BPMs
- Animation stuttering with complex patterns
- Memory leaks during extended playback
- Timing drift with small buffer sizes
- CPU spikes during pattern switching

## Reporting Results
When reporting performance test results, include:
1. DAW and version
2. Audio interface and buffer size
3. System specs (CPU, RAM, OS)
4. Console output logs
5. Specific test scenario that failed
6. Duration of test before failure

---

**Note**: This is a diagnostic version with extensive logging. Production version will have monitoring removed for optimal performance.