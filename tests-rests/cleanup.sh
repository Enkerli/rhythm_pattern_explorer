#!/bin/bash

# Rhythmic Pattern Explorer - Cleanup Script
# This script removes development and testing files, keeping only the clean app

echo "🧹 Rhythmic Pattern Explorer - Directory Cleanup"
echo "================================================"
echo ""
echo "This script will remove development and testing files."
echo "The clean app will remain in the './app/' directory."
echo ""
read -p "Do you want to proceed? (y/N): " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cleanup cancelled."
    exit 1
fi

echo ""
echo "🗑️  Removing development and testing files..."

# Count files before cleanup
TOTAL_FILES=$(find . -maxdepth 1 -type f | wc -l)
echo "📊 Total files before cleanup: $TOTAL_FILES"

# Remove test files
rm -f test-*.html test-*.js
rm -f debug-*.html debug-*.js
rm -f verify-*.html verify-*.js
rm -f integration-tests*.html
rm -f minimal-*.html
rm -f comprehensive-test-suite.html
rm -f simple-test.html
rm -f quick-test.js

# Remove development files
rm -f app-controller.js
rm -f app-template.html
rm -f perfect-balance-analysis.js
rm -f rhythm_pattern_explorer.html

# Remove documentation/analysis files
rm -f "Rhythm Pattern Explorer Checklist.txt"
rm -f *.md

# Remove this cleanup script itself
rm -f cleanup.sh

# Count files after cleanup
REMAINING_FILES=$(find . -maxdepth 1 -type f | wc -l)
APP_FILES=$(find ./app -type f | wc -l)

echo ""
echo "✅ Cleanup complete!"
echo "📊 Files removed: $((TOTAL_FILES - REMAINING_FILES))"
echo "📊 Files remaining in main directory: $REMAINING_FILES"
echo "📊 Clean app files in ./app/: $APP_FILES"
echo ""
echo "🎯 To use the app: Open ./app/index.html in your browser"
echo ""