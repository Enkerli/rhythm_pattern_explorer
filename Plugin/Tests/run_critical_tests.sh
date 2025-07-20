#!/bin/bash

# Critical Test Runner for Xcode Build Integration
# This script runs essential tests before build and fails the build if tests fail

echo "🔬 Running Critical Tests for Build Validation"
echo "=============================================="

# Set up paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="$SCRIPT_DIR"
BUILD_DIR="$TEST_DIR/build"

# Create build directory
mkdir -p "$BUILD_DIR"

# Exit on any error
set -e

echo "📁 Compiling hex/octal notation tests..."

# Compile the core hex/octal tests (no JUCE dependencies)
g++ -std=c++17 -O2 \
    "$TEST_DIR/HexOctalCoreTests.cpp" \
    -o "$BUILD_DIR/hex_octal_core_test"

echo "✅ Compilation successful"
echo ""
echo "🏃 Running critical hex/octal notation tests..."
echo ""

# Run the tests - this will exit with code 1 if tests fail
"$BUILD_DIR/hex_octal_core_test"

echo ""
echo "🎉 ALL CRITICAL TESTS PASSED!"
echo "✅ Build may proceed safely"