#!/bin/bash

# Critical Hex/Octal Notation Test Runner
echo "🔧 Hex/Octal Notation Critical Test Suite"
echo "========================================="

# Set up paths
PLUGIN_DIR="/Users/alex/Documents/Coding/rhythm_pattern_explorer/Plugin"
TEST_DIR="$PLUGIN_DIR/Tests"
SOURCE_DIR="$PLUGIN_DIR/Source"
JUCE_DIR="/Users/alex/JUCE/modules"

# Create test output directory
mkdir -p "$TEST_DIR/build"

echo "📁 Compiling hex/octal test suite..."

# Compile the test suite with all required JUCE modules
clang++ -std=c++17 \
    -I"$SOURCE_DIR" \
    -I"$JUCE_DIR" \
    -I"$PLUGIN_DIR/JuceLibraryCode" \
    -DJUCE_MODULE_AVAILABLE_juce_core=1 \
    -DJUCE_MODULE_AVAILABLE_juce_events=1 \
    -DJUCE_MODULE_AVAILABLE_juce_data_structures=1 \
    -DJUCE_MODULE_AVAILABLE_juce_audio_basics=1 \
    -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 \
    -framework CoreFoundation \
    -framework CoreServices \
    -framework AudioToolbox \
    -O2 \
    "$TEST_DIR/HexOctalNotationTests.cpp" \
    "$SOURCE_DIR/UPIParser.cpp" \
    "$SOURCE_DIR/PatternEngine.cpp" \
    "$SOURCE_DIR/PatternUtils.cpp" \
    -o "$TEST_DIR/build/hex_octal_test_runner"

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful"
    echo ""
    echo "🏃 Running critical hex/octal tests..."
    echo ""
    
    # Run the tests
    "$TEST_DIR/build/hex_octal_test_runner"
    
    test_result=$?
    echo ""
    
    if [ $test_result -eq 0 ]; then
        echo "🎉 ALL HEX/OCTAL TESTS PASSED!"
        echo "✅ Strict left-to-right bit ordering is working correctly"
        exit 0
    else
        echo "❌ HEX/OCTAL TESTS FAILED!"
        echo "⚠️  CRITICAL: Build should fail to prevent regression"
        exit 1
    fi
else
    echo "❌ Compilation failed"
    exit 1
fi