# iPad RPE Critical Tests - Xcode Integration

## Overview

This directory contains critical pattern tests that validate the core algorithms used in the iPad Rhythm Pattern Explorer. These tests ensure that pattern generation works correctly and prevent regressions.

## Test Files

- `CriticalPatternTests.cpp` - Core test suite (no JUCE dependencies)
- `run_critical_tests.sh` - Test runner script
- `XCODE_INTEGRATION.md` - This documentation

## Step-by-Step Xcode Integration

### Step 1: Add Test Files to Xcode Project

1. **Open Xcode Project**: Open `Rhythm Pattern Explorer iPad.xcodeproj`

2. **Add Tests Group**:
   - Right-click on the project root in the navigator
   - Select "New Group"
   - Name it "Tests"

3. **Add Test Files**:
   - Right-click on the "Tests" group
   - Select "Add Files to 'Rhythm Pattern Explorer iPad'"
   - Navigate to `Source/Tests/`
   - Select both `CriticalPatternTests.cpp` and `run_critical_tests.sh`
   - **IMPORTANT**: Uncheck "Add to target" for both files (they shouldn't be compiled with the plugin)
   - Click "Add"

### Step 2: Add Pre-Build Script for AUv3 Target

1. Select the **AUv3 AppExtension** target
2. Go to **Build Phases** tab
3. Click **+** and select **New Run Script Phase**
4. **Drag the script phase to the top** (before "Compile Sources")
5. Set the script name to: `Critical Pattern Tests`
6. Add this script:

```bash
echo "🔬 Running Critical Pattern Tests for AUv3 Build"
cd "${SRCROOT}/Source/Tests"
if [ ! -f "run_critical_tests.sh" ]; then
    echo "❌ Critical test script not found at ${SRCROOT}/Source/Tests"
    echo "📁 Current directory: $(pwd)"
    echo "📋 Available files: $(ls -la)"
    exit 1
fi
chmod +x run_critical_tests.sh
./run_critical_tests.sh
if [ $? -ne 0 ]; then
    echo "❌ CRITICAL TESTS FAILED - AUv3 BUILD ABORTED"
    exit 1
fi
echo "✅ Tests passed - AUv3 build proceeding"
```

### Step 3: Add Pre-Build Script for Standalone Target

Repeat the same process for the **Standalone Plugin** target:

1. Select the **Standalone Plugin** target
2. Follow the same steps as above
3. Change the echo messages to reference "Standalone" instead of "AUv3"

### Step 4: Build Phase Configuration

For both targets, configure the script phase:

- **Shell**: `/bin/sh`
- **Show environment variables in build log**: ✓ (recommended for debugging)
- **Run script only when installing**: ❌ (tests should run for all builds)

## Troubleshooting Build Errors

### "PhaseScriptExecution failed with a nonzero exit code"

This error occurs when:

1. **Test files not found**: The script can't locate the test files
   - **Solution**: Verify the test files are in `Source/Tests/` and added to Xcode
   - Check the console output for the actual path being searched

2. **Tests are failing**: The pattern algorithms are broken
   - **Solution**: Run tests manually to see which patterns fail
   - Fix the underlying pattern generation code

3. **Permission issues**: The script isn't executable
   - **Solution**: The script includes `chmod +x` to fix this automatically

### Manual Test Verification

To debug test issues, run tests manually:

```bash
cd RhythmPatternExplorer_iPad/NewProject/Source/Tests
./run_critical_tests.sh
```

Expected output when tests pass:
```
🎉 ALL CRITICAL TESTS PASSED! 🎉
✅ Core pattern algorithms are working correctly
✅ iPad RPE is ready for build
```

### Path Debugging

If the script can't find files, check the paths:

```bash
# In Xcode build log, look for:
echo "📁 Current directory: $(pwd)"
echo "📋 Available files: $(ls -la)"
```

This will show exactly where Xcode is looking for the test files.

## What the Tests Validate

The critical tests verify:

### Core Euclidean Patterns
- ✅ **Tresillo E(3,8)**: `10010010` (Cuban tresillo rhythm)
- ✅ **Cinquillo E(5,8)**: `10110110` (Cuban cinquillo rhythm)  
- ✅ **E(3,4)**: `1110` (maximally even 3-in-4)

### Basic Pattern Coverage
- ✅ **E(1,4)**: `1000` (single onset)
- ✅ **E(2,4)**: `1010` (even distribution)
- ✅ **E(4,4)**: `1111` (all onsets)

## Benefits

This integration ensures:

- ✅ **No broken builds** - pattern algorithms are validated before compilation
- ✅ **Regression prevention** - changes that break patterns are caught immediately  
- ✅ **Confidence in releases** - all builds are tested for core functionality
- ✅ **Fast feedback** - tests run in seconds, build failures are immediate

The tests run independently of JUCE and complete very quickly, adding minimal overhead to the build process while providing essential validation.

## File Organization

After integration, your project structure should look like:

```
Rhythm Pattern Explorer iPad/
├── Source/
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.cpp
│   ├── PatternUtils.cpp
│   ├── UPIParser.cpp
│   └── Tests/                    ← Test files here
│       ├── CriticalPatternTests.cpp
│       ├── run_critical_tests.sh
│       └── XCODE_INTEGRATION.md
└── [Other project files]
```

The Tests group in Xcode navigator should show both test files, but they should not be included in any build targets.