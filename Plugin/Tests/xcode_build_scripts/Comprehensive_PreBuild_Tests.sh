#!/bin/bash
# Comprehensive Pre-Build Test Script
# For maximum safety - runs full test suite including new accent mapping tests
# Use this version if you want complete test coverage before every build

echo "🔬 Running Comprehensive Test Suite for Build"
echo "=============================================="

# Navigate to Tests directory
cd "${SRCROOT}/Tests" || {
    echo "❌ ERROR: Could not find Tests directory at ${SRCROOT}/Tests"
    exit 1
}

# Run comprehensive test suite using Makefile
echo "Executing comprehensive test suite..."
make test-comprehensive

# Check test results
if [ $? -ne 0 ]; then
    echo ""
    echo "❌ COMPREHENSIVE TESTS FAILED - BUILD ABORTED"
    echo "⚠️  One or more test suites failed:"
    echo "   - Critical fixes validation"
    echo "   - Hex/octal notation tests"
    echo "   - Barlow algorithm tests"
    echo "   - Polygon pattern tests"
    echo "   - Random pattern tests"
    echo "   - Accent mapping tests"
    echo ""
    echo "Fix failing tests before building"
    exit 1
fi

echo ""
echo "✅ All comprehensive tests passed - build proceeding"
echo "✅ Complete system validation successful (216 total tests)"
echo ""