# Critical Test Integration for Rhythm Pattern Explorer Plugin
# Ensures hex/octal notation tests pass before building

.PHONY: test-critical build-with-tests clean

# Default target - run tests then suggest next steps
all: test-critical
	@echo ""
	@echo "🎉 Critical tests passed! You can now build safely:"
	@echo "  make build-au    # Build AU component"
	@echo "  make build-vst3  # Build VST3 plugin"
	@echo "  make build-all   # Build both formats"

# Run critical tests - MUST pass for build to proceed
test-critical:
	@echo "🔬 Running Critical Hex/Octal Notation Tests"
	@echo "============================================="
	@./run_critical_tests.sh

# Build AU with mandatory test validation
build-au: test-critical
	@echo ""
	@echo "🔨 Building AU Component (tests passed)"
	@echo "======================================="
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - AU" -configuration Release build

# Build VST3 with mandatory test validation  
build-vst3: test-critical
	@echo ""
	@echo "🔨 Building VST3 Plugin (tests passed)"
	@echo "======================================"
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - VST3" -configuration Release build

# Build both formats with mandatory test validation
build-all: test-critical
	@echo ""
	@echo "🔨 Building All Plugin Formats (tests passed)"
	@echo "=============================================="
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

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts"
	rm -rf build/
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - AU" -configuration Release clean
	cd ../Builds/MacOSX && xcodebuild -scheme "Rhythm Pattern Explorer - VST3" -configuration Release clean

# Development workflow - test, build, and install in one command
dev: test-critical build-all install
	@echo ""
	@echo "🚀 Development build complete!"
	@echo "✅ Tests passed, plugins built and installed"

# Show help
help:
	@echo "Available targets:"
	@echo "  test-critical  - Run critical hex/octal notation tests"
	@echo "  build-au      - Build AU component (with test validation)"
	@echo "  build-vst3    - Build VST3 plugin (with test validation)"
	@echo "  build-all     - Build both plugin formats (with test validation)"
	@echo "  install       - Install built plugins to user library"
	@echo "  dev           - Complete development workflow (test + build + install)"
	@echo "  clean         - Clean build artifacts"
	@echo "  help          - Show this help message"