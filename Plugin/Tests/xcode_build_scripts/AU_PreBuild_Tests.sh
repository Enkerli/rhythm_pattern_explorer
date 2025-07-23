#!/bin/bash
# AU Pre-Build Test Script
# Add this as a "Run Script Phase" BEFORE "Compile Sources" in AU target Build Phases

echo "🔬 Running Critical Tests for AU Build"
echo "====================================="

# Navigate to Tests directory
cd "${SRCROOT}/Tests" || {
    echo "❌ ERROR: Could not find Tests directory at ${SRCROOT}/Tests"
    exit 1
}

# Check if critical test script exists
if [ ! -f "run_critical_tests.sh" ]; then
    echo "❌ ERROR: Critical test script not found"
    echo "Expected: ${SRCROOT}/Tests/run_critical_tests.sh"
    exit 1
fi

# Make script executable
chmod +x run_critical_tests.sh

# Run critical tests
echo "Running comprehensive test suite..."
./run_critical_tests.sh

# Check test results
if [ $? -ne 0 ]; then
    echo ""
    echo "❌ CRITICAL TESTS FAILED - AU BUILD ABORTED"
    echo "⚠️  The hex/octal notation system has regressions"
    echo "⚠️  Fix failing tests before building"
    echo ""
    exit 1
fi

echo ""
echo "✅ All critical tests passed - AU build proceeding"
echo "✅ Hex/octal notation system validated"
echo ""