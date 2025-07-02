#!/bin/bash

# Rhythm Pattern Explorer - Automated Test Runner
# Version: 1.0
# Usage: ./TestRunner.sh [vst3|au|all]

set -e

PLUGIN_NAME="Rhythm Pattern Explorer"
VST3_PATH="$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"
AU_PATH="$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
TEST_DIR="$(dirname "$0")"
RESULTS_DIR="$TEST_DIR/Results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

# Create results directory
mkdir -p "$RESULTS_DIR"

# Test report file
REPORT_FILE="$RESULTS_DIR/TestReport_${TIMESTAMP}.md"

# Initialize test report
init_report() {
    cat > "$REPORT_FILE" << EOF
# Rhythm Pattern Explorer Test Report

**Date**: $(date)  
**System**: $(sw_vers -productName) $(sw_vers -productVersion)  
**Hardware**: $(sysctl -n hw.model)  
**Plugin Version**: 2.8  

## Test Results Summary

EOF
}

# Check prerequisites
check_prerequisites() {
    log_info "Checking test environment..."
    
    # Check if plugins are installed
    if [ ! -e "$VST3_PATH" ]; then
        log_error "VST3 plugin not found at: $VST3_PATH"
        return 1
    fi
    
    if [ ! -e "$AU_PATH" ]; then
        log_error "AU plugin not found at: $AU_PATH"
        return 1
    fi
    
    # Check for required tools
    if ! command -v auval &> /dev/null; then
        log_warning "auval not found - AU validation tests will be skipped"
    fi
    
    log_success "Environment check passed"
    return 0
}

# Test plugin installation
test_installation() {
    log_info "Testing plugin installation..."
    
    echo "### Installation Tests" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # VST3 installation test
    if [ -e "$VST3_PATH" ]; then
        VST3_SIZE=$(du -h "$VST3_PATH" | cut -f1)
        log_success "VST3 plugin found ($VST3_SIZE)"
        echo "- ✅ VST3 Plugin: Installed ($VST3_SIZE)" >> "$REPORT_FILE"
    else
        log_error "VST3 plugin missing"
        echo "- ❌ VST3 Plugin: Missing" >> "$REPORT_FILE"
    fi
    
    # AU installation test
    if [ -e "$AU_PATH" ]; then
        AU_SIZE=$(du -h "$AU_PATH" | cut -f1)
        log_success "AU plugin found ($AU_SIZE)"
        echo "- ✅ AU Plugin: Installed ($AU_SIZE)" >> "$REPORT_FILE"
    else
        log_error "AU plugin missing"
        echo "- ❌ AU Plugin: Missing" >> "$REPORT_FILE"
    fi
    
    echo "" >> "$REPORT_FILE"
}

# Test AU validation
test_au_validation() {
    log_info "Running AU validation..."
    
    echo "### AU Validation Tests" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    if command -v auval &> /dev/null; then
        log_info "Running auval validation..."
        
        # Capture auval output
        AUVAL_OUTPUT=$(auval -v aufx Rpe1 RPE1 2>&1)
        AUVAL_RESULT=$?
        
        if [ $AUVAL_RESULT -eq 0 ]; then
            log_success "AU validation passed"
            echo "- ✅ auval validation: PASSED" >> "$REPORT_FILE"
        else
            log_error "AU validation failed"
            echo "- ❌ auval validation: FAILED" >> "$REPORT_FILE"
            echo "  \`\`\`" >> "$REPORT_FILE"
            echo "$AUVAL_OUTPUT" >> "$REPORT_FILE"
            echo "  \`\`\`" >> "$REPORT_FILE"
        fi
    else
        log_warning "auval not available - skipping AU validation"
        echo "- ⚠️ auval validation: SKIPPED (tool not available)" >> "$REPORT_FILE"
    fi
    
    echo "" >> "$REPORT_FILE"
}

# Test plugin loading and basic functionality
test_basic_functionality() {
    log_info "Testing basic plugin functionality..."
    
    echo "### Basic Functionality Tests" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # These tests would require DAW automation or plugin hosting framework
    # For now, we'll document the manual test requirements
    
    echo "- ⏳ Plugin Loading: Manual test required" >> "$REPORT_FILE"
    echo "- ⏳ Parameter Response: Manual test required" >> "$REPORT_FILE"
    echo "- ⏳ MIDI Output: Manual test required" >> "$REPORT_FILE"
    echo "- ⏳ Pattern Generation: Manual test required" >> "$REPORT_FILE"
    
    log_warning "Basic functionality tests require manual execution in DAW"
    echo "" >> "$REPORT_FILE"
}

# Performance monitoring test
test_performance() {
    log_info "Running performance tests..."
    
    echo "### Performance Tests" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Test file sizes
    if [ -e "$VST3_PATH" ]; then
        VST3_SIZE_BYTES=$(stat -f%z "$VST3_PATH")
        VST3_SIZE_MB=$((VST3_SIZE_BYTES / 1024 / 1024))
        echo "- VST3 File Size: ${VST3_SIZE_MB}MB" >> "$REPORT_FILE"
        
        if [ $VST3_SIZE_MB -lt 50 ]; then
            log_success "VST3 size acceptable: ${VST3_SIZE_MB}MB"
        else
            log_warning "VST3 size large: ${VST3_SIZE_MB}MB"
        fi
    fi
    
    if [ -e "$AU_PATH" ]; then
        AU_SIZE_BYTES=$(du -sk "$AU_PATH" | cut -f1)
        AU_SIZE_MB=$((AU_SIZE_BYTES / 1024))
        echo "- AU File Size: ${AU_SIZE_MB}MB" >> "$REPORT_FILE"
        
        if [ $AU_SIZE_MB -lt 50 ]; then
            log_success "AU size acceptable: ${AU_SIZE_MB}MB"
        else
            log_warning "AU size large: ${AU_SIZE_MB}MB"
        fi
    fi
    
    # System info
    echo "- CPU: $(sysctl -n machdep.cpu.brand_string)" >> "$REPORT_FILE"
    echo "- Memory: $(sysctl -n hw.memsize | awk '{print $1/1024/1024/1024 " GB"}')" >> "$REPORT_FILE"
    
    echo "" >> "$REPORT_FILE"
}

# Test plugin signatures and security
test_security() {
    log_info "Testing plugin security and signatures..."
    
    echo "### Security and Signing Tests" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Check VST3 signing
    if [ -e "$VST3_PATH" ]; then
        VST3_SIGN_OUTPUT=$(codesign -dv "$VST3_PATH" 2>&1)
        if [[ $VST3_SIGN_OUTPUT == *"Signature"* ]]; then
            log_success "VST3 is code signed"
            echo "- ✅ VST3 Code Signing: Valid" >> "$REPORT_FILE"
        else
            log_warning "VST3 signing status unclear"
            echo "- ⚠️ VST3 Code Signing: Unknown" >> "$REPORT_FILE"
        fi
    fi
    
    # Check AU signing
    if [ -e "$AU_PATH" ]; then
        AU_SIGN_OUTPUT=$(codesign -dv "$AU_PATH" 2>&1)
        if [[ $AU_SIGN_OUTPUT == *"Signature"* ]]; then
            log_success "AU is code signed"
            echo "- ✅ AU Code Signing: Valid" >> "$REPORT_FILE"
        else
            log_warning "AU signing status unclear"
            echo "- ⚠️ AU Code Signing: Unknown" >> "$REPORT_FILE"
        fi
    fi
    
    echo "" >> "$REPORT_FILE"
}

# Generate comprehensive system report
generate_system_report() {
    log_info "Generating system report..."
    
    echo "### System Information" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "- **OS**: $(sw_vers -productName) $(sw_vers -productVersion) ($(sw_vers -buildVersion))" >> "$REPORT_FILE"
    echo "- **Hardware**: $(sysctl -n hw.model)" >> "$REPORT_FILE"
    echo "- **CPU**: $(sysctl -n machdep.cpu.brand_string)" >> "$REPORT_FILE"
    echo "- **Cores**: $(sysctl -n hw.ncpu)" >> "$REPORT_FILE"
    echo "- **Memory**: $(echo "$(sysctl -n hw.memsize) / 1024 / 1024 / 1024" | bc) GB" >> "$REPORT_FILE"
    echo "- **Architecture**: $(uname -m)" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Audio system info
    echo "### Audio System" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Check for common DAWs
    DAWS_FOUND=""
    
    if [ -d "/Applications/Logic Pro.app" ]; then
        LOGIC_VERSION=$(defaults read "/Applications/Logic Pro.app/Contents/Info.plist" CFBundleShortVersionString)
        DAWS_FOUND="$DAWS_FOUND- Logic Pro $LOGIC_VERSION\n"
    fi
    
    if [ -d "/Applications/GarageBand.app" ]; then
        GB_VERSION=$(defaults read "/Applications/GarageBand.app/Contents/Info.plist" CFBundleShortVersionString)
        DAWS_FOUND="$DAWS_FOUND- GarageBand $GB_VERSION\n"
    fi
    
    if [ -d "/Applications/MainStage.app" ]; then
        MS_VERSION=$(defaults read "/Applications/MainStage.app/Contents/Info.plist" CFBundleShortVersionString)
        DAWS_FOUND="$DAWS_FOUND- MainStage $MS_VERSION\n"
    fi
    
    if [ -n "$DAWS_FOUND" ]; then
        echo "**Detected DAWs:**" >> "$REPORT_FILE"
        echo -e "$DAWS_FOUND" >> "$REPORT_FILE"
    else
        echo "- No supported DAWs detected" >> "$REPORT_FILE"
    fi
    
    echo "" >> "$REPORT_FILE"
}

# Main test execution
run_tests() {
    local test_type="$1"
    
    init_report
    
    log_info "Starting test suite: $test_type"
    
    if ! check_prerequisites; then
        log_error "Prerequisites check failed"
        exit 1
    fi
    
    case $test_type in
        "au")
            test_installation
            test_au_validation
            test_basic_functionality
            test_performance
            test_security
            ;;
        "vst3")
            test_installation
            test_basic_functionality
            test_performance
            test_security
            ;;
        "all"|*)
            test_installation
            test_au_validation
            test_basic_functionality
            test_performance
            test_security
            ;;
    esac
    
    generate_system_report
    
    echo "## Manual Testing Required" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "The following tests require manual execution in a DAW:" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "1. **Plugin Loading**: Load in host DAW and verify UI appears" >> "$REPORT_FILE"
    echo "2. **Pattern Generation**: Test UPI patterns like E(3,8), P(5,12)" >> "$REPORT_FILE"
    echo "3. **MIDI Output**: Verify MIDI notes generated correctly" >> "$REPORT_FILE"
    echo "4. **Parameter Control**: Test real-time parameter changes" >> "$REPORT_FILE"
    echo "5. **Progressive Transformations**: Test E(1,8)E>8, E(8,8)B>1" >> "$REPORT_FILE"
    echo "6. **Host Integration**: Test transport sync, automation" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "Refer to VST3_StressTest.md and AU_StressTest.md for detailed procedures." >> "$REPORT_FILE"
    
    log_success "Test suite completed"
    log_info "Report saved to: $REPORT_FILE"
}

# Usage information
show_usage() {
    echo "Usage: $0 [test_type]"
    echo ""
    echo "Test types:"
    echo "  au    - Run AU-specific tests only"
    echo "  vst3  - Run VST3-specific tests only"
    echo "  all   - Run all tests (default)"
    echo ""
    echo "Examples:"
    echo "  $0        # Run all tests"
    echo "  $0 all    # Run all tests"
    echo "  $0 au     # Run AU tests only"
    echo "  $0 vst3   # Run VST3 tests only"
}

# Main execution
main() {
    local test_type="${1:-all}"
    
    case $test_type in
        "au"|"vst3"|"all")
            run_tests "$test_type"
            ;;
        "-h"|"--help"|"help")
            show_usage
            ;;
        *)
            log_error "Unknown test type: $test_type"
            show_usage
            exit 1
            ;;
    esac
}

# Execute main function with all arguments
main "$@"