# Xcode Build Phase Integration for Unit Tests

## Overview

This guide shows how to integrate the critical test suite directly into Xcode Build Phases, ensuring tests run automatically before every build. This prevents builds with broken hex/octal notation or other critical regressions.

## Quick Setup

### Option 1: Critical Tests Only (Recommended)
Use this for fast builds with essential validation:
- **AU Target**: Use `AU_PreBuild_Tests.sh`
- **VST3 Target**: Use `VST3_PreBuild_Tests.sh`
- **Test Coverage**: 48 hex/octal core tests + critical fixes
- **Build Time**: ~2-5 seconds additional

### Option 2: Comprehensive Tests (Maximum Safety)
Use this for complete validation:
- **Both Targets**: Use `Comprehensive_PreBuild_Tests.sh`
- **Test Coverage**: All 216 tests (hex/octal, Barlow, polygon, random, accent mapping)
- **Build Time**: ~10-15 seconds additional

## Detailed Integration Steps

### Step 1: For AU Target (Rhythm Pattern Explorer - AU)

1. **Open Xcode Project**
   ```
   open Plugin/Builds/MacOSX/Rhythm\ Pattern\ Explorer.xcodeproj
   ```

2. **Select AU Target**
   - In the project navigator, click on "Rhythm Pattern Explorer" (project root)
   - In the main editor, select the **"Rhythm Pattern Explorer - AU"** target

3. **Open Build Phases Tab**
   - Click the **"Build Phases"** tab at the top

4. **Add New Run Script Phase**
   - Click the **"+"** button in the top-left
   - Select **"New Run Script Phase"**

5. **Configure the Script Phase**
   - **Name**: Change "Run Script" to **"Critical Tests Validation"**
   - **Shell**: Ensure it's set to `/bin/sh`
   - **Script**: Copy and paste the entire contents of `AU_PreBuild_Tests.sh`

6. **Position the Script Phase**
   - **CRITICAL**: Drag the "Critical Tests Validation" phase to the **TOP** of the list
   - It must be **BEFORE** "Compile Sources"
   - Order should be: Critical Tests Validation → Dependencies → Compile Sources → etc.

7. **Optional Settings**
   - ☑️ Check "Show environment variables in build log" (for debugging)
   - ☐ Leave "Run script only when installing" unchecked (tests should run for all builds)

### Step 2: For VST3 Target (Rhythm Pattern Explorer - VST3)

1. **Select VST3 Target**
   - In the same project, select **"Rhythm Pattern Explorer - VST3"** target

2. **Repeat Steps 3-7** above, but:
   - Use the contents of `VST3_PreBuild_Tests.sh` for the script
   - Name it **"Critical Tests Validation"**

## Alternative: Comprehensive Test Integration

If you want maximum test coverage (all 216 tests), use `Comprehensive_PreBuild_Tests.sh` for both targets instead of the individual AU/VST3 scripts.

**Script Contents for Comprehensive Testing:**
```bash
#!/bin/bash
echo "🔬 Running Comprehensive Test Suite for Build"
echo "=============================================="

cd "${SRCROOT}/Tests" || {
    echo "❌ ERROR: Could not find Tests directory"
    exit 1
}

make test-comprehensive

if [ $? -ne 0 ]; then
    echo "❌ COMPREHENSIVE TESTS FAILED - BUILD ABORTED"
    exit 1
fi

echo "✅ All comprehensive tests passed - build proceeding"
```

## Verifying Integration

### Test the Setup

1. **Make a Harmless Change**
   - Edit any source file (add a comment)
   - Save the file

2. **Build the Project**
   - Press **Cmd+B** or choose Product → Build
   - Watch the build log

3. **Expected Output**
   ```
   🔬 Running Critical Tests for AU Build
   =====================================
   Running comprehensive test suite...
   
   🎉 ALL HEX/OCTAL CORE TESTS PASSED! 🎉
   ✅ All critical tests passed - AU build proceeding
   
   [Normal compilation continues...]
   ```

### Test Failure Scenario

To verify the integration works:

1. **Temporarily Break a Test**
   - Edit `Plugin/Tests/HexOctalCoreTests.cpp`
   - Change an expected value to cause a test failure

2. **Attempt to Build**
   - Press **Cmd+B**
   - **Expected**: Build should fail with clear error message

3. **Fix the Test**
   - Restore the correct expected value
   - Build should now succeed

## Build Log Visibility

The test output will appear in Xcode's build log. To see detailed output:

1. **Show Build Log**
   - View → Navigators → Show Report Navigator (Cmd+9)
   - Click on the latest build

2. **Expand Test Phase**
   - Click the arrow next to "Critical Tests Validation"
   - You'll see the full test output

## Troubleshooting

### Common Issues

**"Could not find Tests directory"**
- Ensure the project's source root is set correctly
- Check that `Tests/` folder exists relative to the .xcodeproj file

**"Critical test script not found"**
- Verify `run_critical_tests.sh` exists in the Tests directory
- Check file permissions: `chmod +x run_critical_tests.sh`

**Tests pass but build still fails**
- Check that the script phase is positioned BEFORE compile phases
- Verify the shell is set to `/bin/sh`

**Build is too slow**
- Switch from comprehensive tests to critical-only tests
- Consider using the Makefile workflow for development and Xcode integration for releases

## Advanced Configuration

### Environment Variables Available

In the script, you have access to:
- `${SRCROOT}` - Path to project source root
- `${PROJECT_DIR}` - Project directory
- `${TARGET_NAME}` - Current target name (AU vs VST3)
- `${BUILT_PRODUCTS_DIR}` - Build output directory

### Custom Test Selection

You can modify the scripts to run specific test suites:

```bash
# Run only critical tests
make test-critical

# Run only accent mapping tests  
make test-accent

# Run only Barlow algorithm tests
make test-barlow
```

## Benefits of Xcode Integration

✅ **Automatic Validation**: Tests run on every build, not just when remembered  
✅ **Team Consistency**: All developers get the same test validation  
✅ **CI/CD Ready**: Build bots will also run tests automatically  
✅ **Fast Feedback**: Immediate notification if changes break critical systems  
✅ **Regression Prevention**: Impossible to build with broken hex/octal notation  

This integration ensures that the critical hex/octal notation system that users depend on cannot be accidentally broken during development.