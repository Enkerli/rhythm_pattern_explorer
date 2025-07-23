#!/bin/bash
# Test script to validate Xcode build integration scripts work correctly
# Run this to verify the scripts will work before adding them to Xcode

echo "Testing Xcode Build Integration Scripts"
echo "======================================"

# Set up environment variables that Xcode would provide
export SRCROOT="/Users/alex/Documents/Coding/rhythm_pattern_explorer/Plugin"

echo "Simulating Xcode environment:"
echo "SRCROOT = ${SRCROOT}"
echo ""

# Test AU script
echo "Testing AU Pre-Build Script..."
echo "------------------------------"
./AU_PreBuild_Tests.sh
AU_RESULT=$?

echo ""
echo "Testing VST3 Pre-Build Script..."
echo "--------------------------------"
./VST3_PreBuild_Tests.sh
VST3_RESULT=$?

echo ""
echo "Testing Comprehensive Pre-Build Script..."
echo "-----------------------------------------"
./Comprehensive_PreBuild_Tests.sh
COMPREHENSIVE_RESULT=$?

echo ""
echo "Integration Test Results:"
echo "========================"
if [ $AU_RESULT -eq 0 ]; then
    echo "✅ AU Script: PASSED"
else
    echo "❌ AU Script: FAILED"
fi

if [ $VST3_RESULT -eq 0 ]; then
    echo "✅ VST3 Script: PASSED"
else
    echo "❌ VST3 Script: FAILED"
fi

if [ $COMPREHENSIVE_RESULT -eq 0 ]; then
    echo "✅ Comprehensive Script: PASSED"
else
    echo "❌ Comprehensive Script: FAILED"
fi

if [ $AU_RESULT -eq 0 ] && [ $VST3_RESULT -eq 0 ] && [ $COMPREHENSIVE_RESULT -eq 0 ]; then
    echo ""
    echo "🎉 ALL INTEGRATION SCRIPTS WORKING!"
    echo "✅ Ready to integrate into Xcode Build Phases"
    echo ""
    echo "Next steps:"
    echo "1. Open Rhythm Pattern Explorer.xcodeproj in Xcode"
    echo "2. Follow instructions in XCODE_BUILD_INTEGRATION.md"
    echo "3. Add scripts to AU and VST3 target Build Phases"
else
    echo ""
    echo "❌ Some integration scripts failed"
    echo "Fix issues before integrating into Xcode"
fi