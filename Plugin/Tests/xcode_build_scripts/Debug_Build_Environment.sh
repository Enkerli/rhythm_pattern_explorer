#!/bin/bash
# Debug Build Environment Script
# Use this temporarily in Xcode to diagnose build phase issues

echo "🔍 Debugging Xcode Build Environment"
echo "===================================="

echo "Environment Variables:"
echo "SRCROOT = ${SRCROOT}"
echo "PROJECT_DIR = ${PROJECT_DIR}"
echo "TARGET_NAME = ${TARGET_NAME}"
echo "PWD = $(pwd)"
echo ""

echo "Directory Listing:"
echo "Current directory: $(pwd)"
ls -la
echo ""

echo "Tests Directory Check:"
if [ -d "${SRCROOT}/Tests" ]; then
    echo "✅ Tests directory found at: ${SRCROOT}/Tests"
    ls -la "${SRCROOT}/Tests/"
else
    echo "❌ Tests directory NOT found at: ${SRCROOT}/Tests"
    echo "Looking for Tests directory..."
    find "${SRCROOT}" -name "Tests" -type d 2>/dev/null || echo "No Tests directory found anywhere in SRCROOT"
fi
echo ""

echo "Script Check:"
if [ -f "${SRCROOT}/Tests/run_critical_tests.sh" ]; then
    echo "✅ Critical test script found"
    ls -la "${SRCROOT}/Tests/run_critical_tests.sh"
else
    echo "❌ Critical test script NOT found at: ${SRCROOT}/Tests/run_critical_tests.sh"
fi
echo ""

echo "Attempting to navigate to Tests directory..."
cd "${SRCROOT}/Tests" 2>/dev/null

if [ $? -eq 0 ]; then
    echo "✅ Successfully navigated to Tests directory"
    echo "Contents:"
    ls -la
    
    echo ""
    echo "Attempting to run critical tests..."
    if [ -f "run_critical_tests.sh" ]; then
        chmod +x run_critical_tests.sh
        ./run_critical_tests.sh
        TEST_RESULT=$?
        echo ""
        echo "Test script exit code: $TEST_RESULT"
        
        if [ $TEST_RESULT -eq 0 ]; then
            echo "✅ Tests passed in Xcode environment"
        else
            echo "❌ Tests failed in Xcode environment"
        fi
    else
        echo "❌ run_critical_tests.sh not found in Tests directory"
    fi
else
    echo "❌ Failed to navigate to Tests directory"
fi

echo ""
echo "Debug complete - check output above for issues"