#!/bin/bash

echo "=== JUCE iPad Plugin Debug Script ==="
echo ""

echo "1. Checking JUCE installation..."
if command -v Projucer &> /dev/null; then
    echo "✅ Projucer found: $(which Projucer)"
else
    echo "❌ Projucer not found in PATH"
    echo "   Please ensure JUCE is properly installed"
fi

echo ""
echo "2. Checking .jucer file..."
JUCER_FILE="RhythmPatternExplorer_iPad.jucer"
if [ -f "$JUCER_FILE" ]; then
    echo "✅ .jucer file exists"
    echo "   File size: $(stat -f%z "$JUCER_FILE") bytes"
    
    # Check for XML validity
    if xmllint --noout "$JUCER_FILE" 2>/dev/null; then
        echo "✅ .jucer file is valid XML"
    else
        echo "❌ .jucer file has XML syntax errors"
        echo "   Running xmllint to show errors:"
        xmllint --noout "$JUCER_FILE"
    fi
else
    echo "❌ .jucer file not found"
fi

echo ""
echo "3. Checking Builds directory..."
if [ -d "Builds" ]; then
    echo "✅ Builds directory exists"
    echo "   Contents:"
    ls -la Builds/
    
    if [ -d "Builds/iOS" ]; then
        echo "✅ iOS build directory exists"
        echo "   Contents:"
        ls -la Builds/iOS/
        
        if [ -d "Builds/iOS/Rhythm Pattern Explorer iPad.xcodeproj" ]; then
            echo "✅ Xcode project exists"
            echo "   Checking project structure..."
            ls -la "Builds/iOS/Rhythm Pattern Explorer iPad.xcodeproj/"
        else
            echo "❌ Xcode project not found"
        fi
    else
        echo "❌ iOS build directory not found"
    fi
else
    echo "❌ Builds directory not found"
fi

echo ""
echo "4. Checking Source files..."
if [ -d "Source" ]; then
    echo "✅ Source directory exists"
    echo "   Contents:"
    ls -la Source/
    
    # Check if all required files exist
    REQUIRED_FILES=("PluginProcessor.h" "PluginProcessor.cpp" "PluginEditor.h" "PluginEditor.cpp")
    for file in "${REQUIRED_FILES[@]}"; do
        if [ -f "Source/$file" ]; then
            echo "✅ $file found"
        else
            echo "❌ $file missing"
        fi
    done
else
    echo "❌ Source directory not found"
fi

echo ""
echo "5. Suggested next steps:"
echo "   - If XML is invalid: Fix .jucer file syntax"
echo "   - If Projucer not found: Install/configure JUCE properly"
echo "   - If Xcode project corrupted: Try manual generation"
echo "   - If all looks good: Try opening in Projucer GUI"

echo ""
echo "=== Debug Complete ==="