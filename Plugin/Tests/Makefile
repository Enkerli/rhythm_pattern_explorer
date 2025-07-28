# Rhythm Pattern Explorer - Comprehensive Test Suite
# Ensures all critical fixes are working before allowing builds

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Test executables
HEX_OCTAL_TESTS = HexOctalCoreTests
BARLOW_TESTS = TestRestoredBarlowAlgorithm
CRITICAL_FIXES_TESTS = CriticalFixesTests
ACCENT_MAPPING_TESTS = AccentMappingTests
ADVANCED_HOST_SYNC_TESTS = AdvancedHostSyncTests
TEST_RUNNER = TestRunner

.PHONY: all test-comprehensive test-fixes test-critical test-barlow test-accent test-host-sync clean build-au build-vst3 dev dev-quick install help

# Default target runs comprehensive test suite
all: test-comprehensive

# Comprehensive test suite with detailed reporting
test-comprehensive: $(TEST_RUNNER)
	@echo "🔬 Running Comprehensive Test Suite"
	@echo "==================================="
	./$(TEST_RUNNER)

# Critical fixes tests (new comprehensive suite)
test-fixes: $(CRITICAL_FIXES_TESTS)
	@echo "🔬 Running Critical Fixes Tests"
	@echo "==============================="
	./$(CRITICAL_FIXES_TESTS)
	@echo "✅ All critical fixes validated!"

# Legacy critical tests (hex/octal) - kept for compatibility
test-critical:
	@echo "🔬 Running Legacy Critical Tests"
	@echo "================================"
	@./run_critical_tests.sh

# Barlow algorithm verification tests
test-barlow: $(BARLOW_TESTS)
	@echo "🔬 Testing Barlow Algorithm"
	@echo "=========================="
	./$(BARLOW_TESTS)

# Accent mapping core logic tests
test-accent: $(ACCENT_MAPPING_TESTS)
	@echo "🔬 Testing Accent Mapping Logic"
	@echo "==============================="
	./$(ACCENT_MAPPING_TESTS)

# Advanced host sync tests (Phase 3)
test-host-sync: $(ADVANCED_HOST_SYNC_TESTS)
	@echo "🔬 Testing Advanced Host Sync Features"
	@echo "======================================"
	./$(ADVANCED_HOST_SYNC_TESTS)

# Build test executables
$(HEX_OCTAL_TESTS): HexOctalCoreTests.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(BARLOW_TESTS): TestRestoredBarlowAlgorithm.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(CRITICAL_FIXES_TESTS): CriticalFixesTests.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(ACCENT_MAPPING_TESTS): AccentMappingTests.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(ADVANCED_HOST_SYNC_TESTS): AdvancedHostSyncTests.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_RUNNER): TestRunner.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

# Plugin builds - require comprehensive tests to pass
build-au: test-comprehensive
	@echo ""
	@echo "🔨 Building AU Component (comprehensive tests passed)"
	@echo "===================================================="
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - AU" -configuration Release build
	@echo "✅ AU plugin build complete!"

build-vst3: test-comprehensive
	@echo ""
	@echo "🔨 Building VST3 Plugin (comprehensive tests passed)"
	@echo "==================================================="  
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - VST3" -configuration Release build
	@echo "✅ VST3 plugin build complete!"

# Build both formats with comprehensive test validation
build-all: test-comprehensive
	@echo ""
	@echo "🔨 Building All Plugin Formats (comprehensive tests passed)"
	@echo "=========================================================="
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - AU" -configuration Release build
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - VST3" -configuration Release build

# Install built plugins to user library
install: build-all
	@echo ""
	@echo "📦 Installing Plugins to User Library"
	@echo "====================================="
	cp -R "../Builds/MacOSX/build/Release/Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/
	cp -R "../Builds/MacOSX/build/Release/Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
	@echo "✅ Plugins installed successfully"

# Development target for comprehensive workflow
dev: test-comprehensive build-all install
	@echo ""
	@echo "🚀 Development build complete with full test coverage!"
	@echo "✅ All tests passed, plugins built and installed"

# Quick development build for debugging (legacy tests only)
dev-quick: test-critical build-vst3
	@echo ""
	@echo "🚀 Quick development build complete!"
	@echo "✅ Legacy tests passed, VST3 plugin built"

# Clean up test executables and build artifacts
clean:
	@echo "🧹 Cleaning test executables and build artifacts"
	rm -f $(HEX_OCTAL_TESTS) $(BARLOW_TESTS) $(CRITICAL_FIXES_TESTS) $(ACCENT_MAPPING_TESTS) $(TEST_RUNNER)
	rm -rf build/
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - AU" -configuration Release clean
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - VST3" -configuration Release clean

# Show help
help:
	@echo "Available targets:"
	@echo ""
	@echo "Testing:"
	@echo "  test-comprehensive - Run full test suite with detailed reporting (default)"
	@echo "  test-fixes         - Run critical fixes validation tests"
	@echo "  test-critical      - Run legacy hex/octal tests (compatibility)"
	@echo "  test-barlow        - Run Barlow algorithm verification tests"
	@echo "  test-accent        - Run accent mapping core logic tests"
	@echo ""
	@echo "Building:"
	@echo "  build-au           - Build AU component (comprehensive tests required)"
	@echo "  build-vst3         - Build VST3 plugin (comprehensive tests required)"
	@echo "  build-all          - Build both plugin formats (comprehensive tests required)"
	@echo ""
	@echo "Development:"
	@echo "  dev                - Full workflow: test + build + install (recommended)"
	@echo "  dev-quick          - Quick workflow: legacy tests + VST3 build only"
	@echo "  install            - Install built plugins to user library"
	@echo ""
	@echo "Maintenance:"
	@echo "  clean              - Remove test executables and build artifacts"
	@echo "  help               - Show this help message"
	@echo ""
	@echo "🔬 Test-driven development: All builds require passing tests to prevent regressions"