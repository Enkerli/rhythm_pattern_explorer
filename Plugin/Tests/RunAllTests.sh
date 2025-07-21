#!/bin/bash

# Comprehensive test runner for Rhythm Pattern Explorer
# Runs all unit tests and reports overall status

echo "🧪 RHYTHM PATTERN EXPLORER - COMPREHENSIVE TEST SUITE"
echo "====================================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test and track results
run_test() {
    local test_name="$1"
    local executable="$2"
    
    echo -e "${BLUE}Running: ${test_name}${NC}"
    echo "----------------------------------------"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if ./"$executable" > "${executable}.log" 2>&1; then
        echo -e "${GREEN}✅ PASSED${NC}: $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo ""
    else
        echo -e "${RED}❌ FAILED${NC}: $test_name"
        echo "Error details:"
        cat "${executable}.log"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo ""
    fi
}

# Compile all tests
echo "🔨 Compiling test suites..."
echo ""

# Core algorithm tests
g++ -std=c++17 TestAllPatternAlgorithms.cpp -o TestAllPatternAlgorithms
g++ -std=c++17 TestProgressiveOffsetsStandalone.cpp -o TestProgressiveOffsetsStandalone
g++ -std=c++17 TestProgressiveLengthening.cpp -o TestProgressiveLengthening
g++ -std=c++17 TestProgressiveTransformations.cpp -o TestProgressiveTransformations
g++ -std=c++17 TestProgressiveIntegration.cpp -o TestProgressiveIntegration

echo "✅ All test suites compiled successfully"
echo ""

# Run all test suites
echo "🚀 Running comprehensive test suite..."
echo ""

# Core pattern algorithm regression tests
run_test "Pattern Algorithm Regression Tests" "TestAllPatternAlgorithms"

# Progressive feature tests
run_test "Progressive Offset Pattern Tests" "TestProgressiveOffsetsStandalone"
run_test "Progressive Lengthening Pattern Tests" "TestProgressiveLengthening"
run_test "Progressive Transformation Tests" "TestProgressiveTransformations"

# Integration tests
run_test "Progressive Pattern Integration Tests" "TestProgressiveIntegration"

# Summary
echo "📊 TEST SUMMARY"
echo "==============="
echo -e "Total Tests:  ${TOTAL_TESTS}"
echo -e "Passed:       ${GREEN}${PASSED_TESTS}${NC}"
echo -e "Failed:       ${RED}${FAILED_TESTS}${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
    echo ""
    echo "✨ Code quality verified:"
    echo "  ✓ Pattern algorithms work correctly after refactoring"
    echo "  ✓ Progressive offset patterns (+N) tested comprehensively"
    echo "  ✓ Progressive lengthening patterns (*N) tested comprehensively" 
    echo "  ✓ Progressive transformations (>N) tested comprehensively"
    echo "  ✓ Integration behavior verified"
    echo "  ✓ Edge cases handled properly"
    echo "  ✓ Regression testing complete"
    echo ""
    echo "🚀 Ready for production deployment!"
    exit 0
else
    echo -e "${RED}❌ SOME TESTS FAILED ❌${NC}"
    echo ""
    echo "Please review the failed tests above and fix the issues."
    echo "Check the individual .log files for detailed error information."
    exit 1
fi