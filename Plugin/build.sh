#!/bin/bash

# Rhythm Pattern Explorer AUv3 Plugin Build Script
# Requires JUCE framework to be available

set -e

echo "🎹 Building Rhythm Pattern Explorer AUv3 Plugin..."

# Check if JUCE is available
if ! command -v Projucer &> /dev/null; then
    echo "❌ Projucer not found. Please ensure JUCE is installed and Projucer is in PATH"
    echo "💡 You can download JUCE from: https://juce.com/get-juce"
    exit 1
fi

# Check if .jucer file exists
if [ ! -f "RhythmPatternExplorer.jucer" ]; then
    echo "❌ RhythmPatternExplorer.jucer not found"
    exit 1
fi

echo "✅ Found Projucer and .jucer file"

# Generate Xcode project
echo "📦 Generating Xcode project with Projucer..."
Projucer --resave RhythmPatternExplorer.jucer

# Check if Xcode project was generated
if [ ! -d "Builds/MacOSX" ]; then
    echo "❌ Failed to generate Xcode project"
    exit 1
fi

echo "✅ Xcode project generated successfully"

# Try to build for macOS (if xcodebuild is available)
if command -v xcodebuild &> /dev/null; then
    echo "🔨 Building macOS version..."
    cd Builds/MacOSX
    
    # Build debug version
    xcodebuild -project "Rhythm Pattern Explorer.xcodeproj" -scheme "Rhythm Pattern Explorer - AU" -configuration Debug build
    
    if [ $? -eq 0 ]; then
        echo "✅ macOS Debug build successful!"
    else
        echo "⚠️ macOS Debug build failed, but project generation worked"
    fi
    
    cd ../..
else
    echo "⚠️ xcodebuild not found - project generated but not built"
fi

# Check for iOS project
if [ -d "Builds/iOS" ]; then
    echo "✅ iOS project also generated"
    
    if command -v xcodebuild &> /dev/null; then
        echo "🔨 Testing iOS build..."
        cd Builds/iOS
        
        # Test iOS build (simulator)
        xcodebuild -project "Rhythm Pattern Explorer.xcodeproj" -scheme "Rhythm Pattern Explorer - AUv3 Plugin" -configuration Debug -sdk iphonesimulator build
        
        if [ $? -eq 0 ]; then
            echo "✅ iOS build successful!"
        else
            echo "⚠️ iOS build failed, but project generation worked"
        fi
        
        cd ../..
    fi
fi

echo ""
echo "🎉 Build process completed!"
echo ""
echo "📁 Generated projects:"
echo "   • macOS: Builds/MacOSX/"
echo "   • iOS:   Builds/iOS/"
echo ""
echo "🎯 Next steps:"
echo "   1. Open the Xcode project in Builds/MacOSX/ or Builds/iOS/"
echo "   2. Set your development team for iOS builds"
echo "   3. Build and test the plugin"
echo "   4. For AUv3 testing, install on device and use in compatible hosts"
echo ""
echo "🧪 Test hosts for AUv3:"
echo "   • GarageBand (iOS/macOS)"
echo "   • Cubasis (iOS)"
echo "   • AUM (iOS)"
echo "   • Logic Pro (macOS)"
echo "   • MainStage (macOS)"