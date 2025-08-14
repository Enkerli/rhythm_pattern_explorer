#!/bin/bash

# Critical Test Runner for iPad RPE Build Integration
# This script runs essential tests before build and fails the build if tests fail

echo "🔬 Running Critical Tests for iPad RPE Build"
echo "============================================"

# Set up paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="$SCRIPT_DIR"
BUILD_DIR="$TEST_DIR/build"

# Create build directory
mkdir -p "$BUILD_DIR"

# Exit on any error
set -e

echo "📁 Compiling critical pattern tests..."

# Compile the core pattern tests (no JUCE dependencies)
g++ -std=c++17 -O2 \
    "$TEST_DIR/CriticalPatternTests.cpp" \
    -o "$BUILD_DIR/critical_pattern_test"

echo "✅ Compilation successful"
echo ""
echo "🏃 Running critical pattern tests..."
echo ""

# Run the tests - this will exit with code 1 if tests fail
"$BUILD_DIR/critical_pattern_test"

echo ""
echo "🎉 ALL CRITICAL TESTS PASSED!"
echo "✅ iPad RPE build may proceed safely"