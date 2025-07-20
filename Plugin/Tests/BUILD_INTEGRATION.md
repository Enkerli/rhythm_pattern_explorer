# Build Integration for Critical Tests

## Overview

This document describes how to integrate critical tests into the build system to prevent regressions in the hex/octal notation system.

## Critical Test: HexOctalCoreTests.cpp

The `HexOctalCoreTests.cpp` file contains comprehensive tests for the strict left-to-right bit ordering system that is essential to the plugin's notation. **These tests MUST pass for all builds.**

### Why These Tests Are Critical

The hex/octal notation system uses a unique approach:
- **LSB-first bit ordering**: Leftmost bit = least significant bit
- **Digit-reversed input parsing**: Hex digits processed right-to-left
- **Normal display order**: Standard left-to-right digit display

This system is non-standard and fragile. Any changes to the parsing or display logic can break user patterns and cause data loss.

### User Examples That Must Work

The tests validate these critical user examples:
- `0x94:8` → `10010010` (tresillo pattern) → displays as `0x94`
- `0x18:8` → `10000001` → displays as `0x18`
- `0x12:8` → `10000100` → displays as `0x12`
- `0x4:4` → `0010` (not `0100`) → displays as `0x4`
- `o12:6` → `100010` → displays as `o12`

## Running the Tests

### Manual Test Execution

```bash
cd Plugin/Tests
g++ -std=c++17 -O2 HexOctalCoreTests.cpp -o build/hex_octal_core_test
./build/hex_octal_core_test
```

### Expected Output (Success)

```
=== Hex/Octal Core Logic Test Suite ===
Testing strict left-to-right bit ordering (LSB-first, digit-reversed)

--- Hex Round-Trip Tests (Critical) ---
✓ [All tests pass]

--- Octal Round-Trip Tests ---
✓ [All tests pass]

--- User-Provided Examples (Regression Prevention) ---
✓ [All tests pass]

--- Edge Cases ---
✓ [All tests pass]

=== Hex/Octal Core Test Suite Summary ===
Tests Run: 48
Passed: 48
Failed: 0
🎉 ALL HEX/OCTAL CORE TESTS PASSED! 🎉
✅ Strict left-to-right bit ordering is working correctly
✅ LSB-first, digit-reversed notation is preserved
Success rate: 100%
```

### Build Failure (Test Failure)

If any test fails, the test runner exits with code 1, which should fail the build:

```
❌ [X] tests failed
⚠️  CRITICAL: Hex/Octal notation system is BROKEN
⚠️  This will cause user confusion and data loss
```

## Xcode Integration Options

### Option 1: Pre-Build Script (Recommended)

Add a "Run Script" build phase at the beginning of the build process:

1. Open `Rhythm Pattern Explorer.xcodeproj` in Xcode
2. Select the **AU** target (`Rhythm Pattern Explorer - AU`)
3. Go to **Build Phases** tab
4. Click **+** and select **New Run Script Phase**
5. **Drag the script phase to the top** (before "Compile Sources")
6. Set the script name to: `Critical Tests Validation`
7. Add this script:

```bash
echo "🔬 Running Critical Tests for AU Build"
cd "${SRCROOT}/Tests"
if [ ! -f "run_critical_tests.sh" ]; then
    echo "❌ Critical test script not found"
    exit 1
fi
chmod +x run_critical_tests.sh
./run_critical_tests.sh
if [ $? -ne 0 ]; then
    echo "❌ CRITICAL TESTS FAILED - AU BUILD ABORTED"
    exit 1
fi
echo "✅ Tests passed - AU build proceeding"
```

8. **Repeat steps 2-7 for VST3 target** (`Rhythm Pattern Explorer - VST3`)
   - Change the echo message to "Running Critical Tests for VST3 Build"
   - Change the failure message to "VST3 BUILD ABORTED"

### Xcode Build Phase Configuration Details

**Shell**: `/bin/sh`
**Show environment variables in build log**: ✓ (recommended for debugging)
**Run script only when installing**: ❌ (tests should run for all builds)

This ensures that **every build attempt** will first validate the critical hex/octal notation system.

### Option 2: Make Integration (Available Now)

A complete Makefile is provided in `/Plugin/Tests/Makefile` with test integration:

```bash
# Run tests only
cd Plugin/Tests && make test-critical

# Build with mandatory test validation
cd Plugin/Tests && make build-au      # AU component
cd Plugin/Tests && make build-vst3    # VST3 plugin  
cd Plugin/Tests && make build-all     # Both formats

# Complete development workflow
cd Plugin/Tests && make dev           # test + build + install
```

The Makefile **refuses to build** if tests fail, ensuring the hex/octal notation system stays working.

### Option 3: CI/CD Integration

For GitHub Actions or similar:

```yaml
- name: Run Critical Tests
  run: |
    cd Plugin/Tests
    g++ -std=c++17 -O2 HexOctalCoreTests.cpp -o build/test
    ./build/test
```

## Test Coverage

The test suite covers:

### Core Functionality
- Round-trip hex parsing and display
- Round-trip octal parsing and display
- LSB-first bit ordering
- Digit-reversed input parsing
- Normal display order

### User Examples
- All user-provided examples that prompted the notation system
- Specific patterns that were broken in previous versions
- Edge cases that caused confusion

### Regression Prevention
- Tests that would catch common mistakes:
  - Reversing bit order back to MSB-first
  - Changing digit processing order
  - Breaking display formatting
  - Incorrect zero-padding handling

## Maintenance

### Adding New Tests

When adding new hex/octal functionality:

1. Add test cases to `HexOctalCoreTests.cpp`
2. Include user examples if reported issues
3. Test both input parsing and display output
4. Verify round-trip consistency

### Test Failure Investigation

If tests fail after code changes:

1. **DO NOT merge/build** until tests pass
2. Check if changes affect:
   - `UPIParser::parseNumericPattern()` 
   - `PatternUtils::getHexString()`
   - `PatternUtils::getOctalString()`
   - `UPIParser::parseDecimal()`
3. Verify user examples still work manually
4. Consider if the change is intentional (update tests) or a bug (fix code)

## Files Involved

### Test Files
- `HexOctalCoreTests.cpp` - Core test suite (no dependencies)
- `HexOctalNotationTests.cpp` - Full test suite (requires JUCE)
- `BUILD_INTEGRATION.md` - This documentation

### Source Files Under Test
- `Plugin/Source/UPIParser.cpp` - Input parsing logic
- `Plugin/Source/PatternUtils.cpp` - Display generation logic

### Documentation
- `CLAUDE.md` - Complete notation system documentation
- `README.md` - User-facing notation examples