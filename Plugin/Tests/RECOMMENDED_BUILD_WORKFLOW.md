# Recommended Build Workflow with Integrated Testing

## Overview

The recommended build approach uses the comprehensive Makefile in `Plugin/Tests/` which ensures all critical tests pass before allowing builds to proceed. This prevents regressions and maintains code quality.

## Quick Start

```bash
# Full development workflow (recommended)
cd Plugin/Tests && make dev

# This will:
# 1. Run comprehensive test suite 
# 2. Build both AU and VST3 plugins
# 3. Install to user library
```

## Build Targets

### Comprehensive Testing (Recommended)

```bash
cd Plugin/Tests

# Run all tests with detailed reporting
make test-comprehensive

# Build with mandatory test validation  
make build-all        # Both AU and VST3
make build-au         # AU component only
make build-vst3       # VST3 plugin only
```

### Development Workflows

```bash
# Full workflow: test + build + install (recommended)
make dev

# Quick workflow: legacy tests + VST3 only
make dev-quick
```

### Manual Testing

```bash
# Run specific test suites
make test-fixes       # Critical fixes validation
make test-critical    # Legacy hex/octal tests  
make test-barlow      # Barlow algorithm tests
```

## Test Coverage

The integrated test suite validates:

### Critical Systems
- **Hex/Octal Notation**: 48 tests ensuring LSB-first, digit-reversed notation works correctly
- **Barlow Algorithm**: Hierarchical indispensability validation  
- **Pattern Manipulation**: Core pattern generation and transformation logic
- **Critical Fixes**: All major bug fixes from July 2025 development

### User-Facing Features
- Round-trip pattern parsing and display
- User-provided examples that must work
- Edge cases that caused previous regressions
- Algorithm correctness for all pattern types

## Safety Guarantees

**All build targets refuse to proceed if tests fail**, ensuring:
- No regressions in user-facing notation
- Algorithm correctness preserved  
- Critical fixes remain working
- Code quality maintained

## Integration with Xcode

While Xcode can be used directly for development, **the Makefile workflow is recommended for production builds** because:

1. **Comprehensive test coverage** - All critical systems validated
2. **Regression prevention** - Builds fail if tests fail
3. **Consistent results** - Same workflow across different environments
4. **Documentation** - Clear target descriptions and help system

## For New Developers

```bash
# First time setup
cd Plugin/Tests
make help              # See all available targets
make test-comprehensive # Verify tests pass
make dev               # Build and install plugins

# Daily development
make dev               # Always use this for safety
```

## Build Artifacts

- **AU Component**: `~/Library/Audio/Plug-Ins/Components/Rhythm Pattern Explorer.component`
- **VST3 Plugin**: `~/Library/Audio/Plug-Ins/VST3/Rhythm Pattern Explorer.vst3`
- **Test Results**: Detailed console output showing all test results

This workflow ensures that every build is validated against the critical test suite, preventing the notation system regressions and algorithm bugs that occurred in previous development phases.