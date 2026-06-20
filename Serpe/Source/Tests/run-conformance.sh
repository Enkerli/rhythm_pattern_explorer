#!/usr/bin/env bash
# Build & run the rhythm codec conformance test (encode/display side).
#
# Compiles PatternUtils.cpp + RhythmConformanceTests.cpp against juce_core only
# (PatternUtils' codecs need nothing heavier) and runs the vectors. No DAW, no
# Projucer step. Exit 0 = the plugin's display codecs match the suite contract.
#
#   Serpe/Source/Tests/run-conformance.sh
#
# Override the JUCE modules path if it isn't the default:
#   JUCE_MODULES=/path/to/JUCE/modules Serpe/Source/Tests/run-conformance.sh
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../../.." && pwd)"
JUCE_MODULES="${JUCE_MODULES:-/Applications/JUCE/modules}"
JLC="$REPO/Serpe/JuceLibraryCode"

if [[ ! -d "$JUCE_MODULES" ]]; then
  echo "JUCE modules not found at: $JUCE_MODULES" >&2
  echo "Set JUCE_MODULES=/path/to/JUCE/modules and retry." >&2
  exit 2
fi

# Keep the generated vectors header in sync if node is available (optional).
if command -v node >/dev/null 2>&1; then
  node "$HERE/gen-conformance-header.mjs" >/dev/null
fi

OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

# juce_core takes ~15s to compile; cache it so repeated runs (e.g. an Xcode
# post-build phase on every rebuild) are cheap. Only juce_core is cached — the
# small TUs recompile each run so source edits are always picked up.
CACHE="$REPO/Serpe/Builds/.conformance-cache"
mkdir -p "$CACHE"

DEFS=(-DNDEBUG=1 -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_STANDALONE_APPLICATION=1 -DJUCE_WEB_BROWSER=0 -DJUCE_USE_CURL=0)
INC=(-I"$JLC" -I"$JUCE_MODULES")
FRAMEWORKS=(-framework Cocoa -framework Foundation -framework IOKit -framework Security)

if [[ ! -f "$CACHE/juce_core.o" || ! -f "$CACHE/juce_core_ct.o" ]]; then
  echo "Compiling juce_core (cached after first run)…"
  clang++ -std=c++17 -O1 "${DEFS[@]}" "${INC[@]}" \
    -ObjC++ -c "$JLC/include_juce_core.mm" -o "$CACHE/juce_core.o"
  clang++ -std=c++17 -O1 "${DEFS[@]}" "${INC[@]}" \
    -c "$JLC/include_juce_core_CompilationTime.cpp" -o "$CACHE/juce_core_ct.o"
fi

echo "Compiling PatternUtils + test…"
clang++ -std=c++17 -O1 "${DEFS[@]}" "${INC[@]}" \
  -c "$REPO/Serpe/Source/Core/PatternUtils.cpp" -o "$OUT/PatternUtils.o"
clang++ -std=c++17 -O1 "${DEFS[@]}" "${INC[@]}" \
  -c "$HERE/RhythmConformanceTests.cpp" -o "$OUT/test.o"
clang++ -std=c++17 "$CACHE/juce_core.o" "$CACHE/juce_core_ct.o" "$OUT"/*.o \
  "${FRAMEWORKS[@]}" -o "$OUT/conformance"

echo "Running…"
"$OUT/conformance"
