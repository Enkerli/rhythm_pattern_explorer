#\!/bin/bash

# Comprehensive UPI Parser Test Runner
echo "🧪 UPI Parser Test Suite Runner"
echo "==============================="

# Set up paths
PLUGIN_DIR="/Users/alex/Documents/Coding/rhythm_pattern_explorer/Plugin"
TEST_DIR="$PLUGIN_DIR/Tests"
SOURCE_DIR="$PLUGIN_DIR/Source"
JUCE_DIR="/Users/alex/JUCE/modules"

# Create test output directory
mkdir -p "$TEST_DIR/build"

echo "📁 Compiling test suite..."

# Compile the test suite
clang++ -std=c++17 \
    -I"$SOURCE_DIR" \
    -I"$JUCE_DIR" \
    -I"$PLUGIN_DIR/JuceLibraryCode" \
    -DJUCE_MODULE_AVAILABLE_juce_core=1 \
    -DJUCE_MODULE_AVAILABLE_juce_events=1 \
    -DJUCE_MODULE_AVAILABLE_juce_data_structures=1 \
    -framework CoreFoundation \
    -framework CoreServices \
    -O2 \
    "$TEST_DIR/UPIParserTests.cpp" \
    "$SOURCE_DIR/UPIParser.cpp" \
    "$SOURCE_DIR/PatternEngine.cpp" \
    -o "$TEST_DIR/build/test_runner"

if [ $? -eq 0 ]; then
    echo "✅ Compilation successful"
    echo ""
    echo "🏃 Running tests..."
    echo ""
    
    # Run the tests
    "$TEST_DIR/build/test_runner"
    
    test_result=$?
    echo ""
    
    if [ $test_result -eq 0 ]; then
        echo "🎉 All tests completed successfully\!"
    else
        echo "❌ Some tests failed (exit code: $test_result)"
    fi
else
    echo "❌ Compilation failed"
    exit 1
fi
EOF < /dev/null