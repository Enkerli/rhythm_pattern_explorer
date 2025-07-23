#!/bin/bash
# Simplified AU Test Script with Enhanced Error Checking
# Use this version if the original script fails in Xcode

set -e  # Exit on any error

echo "🔬 Running Critical Tests for AU Build (Enhanced)"
echo "================================================"

# Debug: Show environment
echo "Debug Info:"
echo "  SRCROOT = ${SRCROOT}"
echo "  PWD = $(pwd)"
echo ""

# Find the correct path to Tests directory
TESTS_DIR=""
if [ -d "${SRCROOT}/Tests" ]; then
    TESTS_DIR="${SRCROOT}/Tests"
elif [ -d "${SRCROOT}/../Tests" ]; then
    TESTS_DIR="${SRCROOT}/../Tests"
elif [ -d "$(dirname ${SRCROOT})/Tests" ]; then
    TESTS_DIR="$(dirname ${SRCROOT})/Tests"
else
    echo "❌ ERROR: Cannot find Tests directory"
    echo "Searched:"
    echo "  ${SRCROOT}/Tests"
    echo "  ${SRCROOT}/../Tests"
    echo "  $(dirname ${SRCROOT})/Tests"
    exit 1
fi

echo "✅ Found Tests directory: ${TESTS_DIR}"

# Navigate to Tests directory
cd "${TESTS_DIR}" || {
    echo "❌ ERROR: Could not navigate to Tests directory: ${TESTS_DIR}"
    exit 1
}

echo "✅ Successfully navigated to Tests directory"

# Check for test script
if [ ! -f "run_critical_tests.sh" ]; then
    echo "❌ ERROR: Critical test script not found"
    echo "Looking for: $(pwd)/run_critical_tests.sh"
    echo "Directory contents:"
    ls -la
    exit 1
fi

echo "✅ Found critical test script"

# Make script executable
chmod +x run_critical_tests.sh

# Run tests with explicit error handling
echo "Running tests..."
./run_critical_tests.sh

# Check result explicitly
if [ $? -eq 0 ]; then
    echo ""
    echo "✅ All tests passed - AU build proceeding"
    exit 0
else
    echo ""
    echo "❌ Tests failed - AU build aborted"
    exit 1
fi