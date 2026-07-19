cmake_minimum_required(VERSION 3.22)
project(Serpe VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# ── Suite foundation (enkerli-juce submodule): JUCE resolution + archetypes ──
# enkerli-juce is a git submodule — a plain `git clone` leaves it empty and
# the include below then fails with a cryptic "Unknown CMake command".
# Initialize it automatically so fresh clones just work.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        message(STATUS "enkerli-juce submodule is empty — running git submodule update --init")
        execute_process(COMMAND "${GIT_EXECUTABLE}" submodule update --init enkerli-juce
                        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                        RESULT_VARIABLE _submod_rc)
    endif()
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
        message(FATAL_ERROR "enkerli-juce submodule missing. Run: git submodule update --init")
    endif()
endif()
include("${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
enkerli_resolve_juce()

# Per-machine signing override (gitignored). The Apple team is account- and
# machine-specific, so it is NOT committed — pinning the wrong one here makes
# Xcode show "unknown account". Without an override, Xcode auto-fills the
# machine's default account. To pin a team, copy signing.local.cmake.example to
# signing.local.cmake and set it there. Must precede enkerli_add_*_plugin so the
# Xcode attribute reaches every target (SharedCode, Standalone, AUv3, install).
include("${CMAKE_CURRENT_SOURCE_DIR}/signing.local.cmake" OPTIONAL)

# ── Plugin (midi_effect / aumi archetype) ───────────────────────────────────
# Identity is load-bearing: PLUGIN_CODE RPEd, manufacturer Enke, bundle
# com.enkerli.serpe — installed hosts key saved sessions on these. The
# archetype would lowercase the bundle id, so it is passed explicitly.
enkerli_add_midi_effect_plugin(Serpe
    PLUGIN_CODE RPEd
    BUNDLE_ID   "com.enkerli.serpe"
    VERSION     "${PROJECT_VERSION}"
    DESCRIPTION "Mathematical rhythm pattern generator and analyzer"
    # macOS .icns / desktop hosts need the corners baked in (JUCE does NOT round
    # them), so point ICON_BIG at the rounded master — matches PitchFold and the
    # rest of the suite. (The full-bleed/-dark/-tinted masters stay in Assets/icon
    # for the iPadOS-26 AUv3 per-appex asset catalog — Any/Dark/Tinted — which is
    # a separate iOS-packaging task; see DESIGN_HANDOFF §7.)
    ICON_BIG    "${CMAKE_SOURCE_DIR}/Assets/icon/serpe-1024.png"
)

# The existing sources include <JuceHeader.h> (Projucer style); generate it so
# they compile unchanged under CMake.
juce_generate_juce_header(Serpe)

add_compile_options($<$<CXX_COMPILER_ID:GNU,Clang>:-finput-charset=UTF-8>)

# ── Sources (UI is now the React WebView editor; the native PluginEditor is
#    retired — kept on disk as attic but no longer compiled) ──
target_sources(Serpe PRIVATE
    Source/Core/PatternEngine.cpp
    Source/Core/PatternUtils.cpp
    Source/Core/QuantizationEngine.cpp
    Source/Core/UPIParser.cpp
    Source/Core/AccentSequence.cpp
    Source/Core/PolyParser.cpp
    Source/Managers/PresetManager.cpp
    Source/Managers/ProgressiveManager.cpp
    Source/Managers/SceneManager.cpp
    Source/Platform/PluginProcessor.cpp
    Source/WebUI/SerpeEditor.cpp
    # Header-only — listed so they show in the Xcode navigator
    Source/Core/PatternEngine.h
    Source/Core/PatternUtils.h
    Source/Core/QuantizationEngine.h
    Source/Core/UPIParser.h
    Source/Core/AccentSequence.h
    Source/Core/PolyParser.h
    Source/Core/PolyClock.h
    Source/Managers/PresetManager.h
    Source/Managers/ProgressiveManager.h
    Source/Managers/SceneManager.h
    Source/Platform/PluginProcessor.h
    Source/WebUI/SerpeEditor.h
    Source/Platform/PlatformSpecific.h
    Source/Platform/DebugConfig.h
)

target_include_directories(Serpe PRIVATE
    Source Source/Core Source/Managers Source/Platform Source/WebUI)

# ── WebUI: bundle the React app (esbuild) and embed it as binary data ─────────
# esbuild produces a SINGLE output (bundle.js) into the build tree; CSS and the
# icon are inlined into it (--loader:.css=text / .svg=dataurl). index.html is a
# static source consumed directly. One generated output + static inputs avoids
# the Xcode build-graph cycle that multi-output custom commands trigger.
#
# The WebUI *source* lives in the music-suite monorepo (apps/serpe) — single
# source shared with the standalone webapp; only the C++ editor (SerpeEditor.*)
# stays here in Source/WebUI. Default to the maintainer's sibling checkout; if
# your layout differs, set SERPE_WEBUI_DIR in webui.local.cmake (gitignored).
# esbuild + react resolve from the monorepo's hoisted node_modules, so the
# monorepo must be checked out and `npm install`ed for the plugin build.
set(SERPE_WEBUI_DIR "${CMAKE_SOURCE_DIR}/../music-suite/apps/serpe"
    CACHE PATH "Path to the Serpe WebUI source (music-suite/apps/serpe)")
include("${CMAKE_CURRENT_SOURCE_DIR}/webui.local.cmake" OPTIONAL)
# The default above assumes music-suite is a SIBLING checkout. Before failing,
# probe the other supported layouts: this repo nested INSIDE the monorepo
# (…/music-suite/Serpe), and a MUSIC_SUITE environment variable pointing at
# the monorepo checkout. webui.local.cmake still wins when present.
if(NOT EXISTS "${SERPE_WEBUI_DIR}/main.jsx")
    foreach(_webui_cand "${CMAKE_SOURCE_DIR}/../apps/serpe" "$ENV{MUSIC_SUITE}/apps/serpe")
        if(EXISTS "${_webui_cand}/main.jsx")
            set(SERPE_WEBUI_DIR "${_webui_cand}" CACHE PATH
                "Path to the Serpe WebUI source (music-suite/apps/serpe)" FORCE)
            message(STATUS "Serpe WebUI: found at ${_webui_cand}")
            break()
        endif()
    endforeach()
endif()
set(WEBUI_DIR   "${SERPE_WEBUI_DIR}")
set(WEBUI_BUILD "${CMAKE_CURRENT_BINARY_DIR}/webui")

if(NOT EXISTS "${WEBUI_DIR}/main.jsx")
    message(FATAL_ERROR
        "Serpe WebUI source not found at ${WEBUI_DIR}. The WebUI now lives in the "
        "music-suite monorepo (apps/serpe). Check out music-suite next to this repo, "
        "or set SERPE_WEBUI_DIR in webui.local.cmake, or export MUSIC_SUITE=/path/to/music-suite. Layouts probed automatically: sibling checkout (../music-suite) and this repo nested inside the monorepo (../apps/serpe) (see webui.local.cmake.example).")
endif()

find_program(NPXCMD npx)
if(NPXCMD)
    file(GLOB_RECURSE WEBUI_SRC
        "${WEBUI_DIR}/main.jsx" "${WEBUI_DIR}/*.js"
        "${WEBUI_DIR}/engine/*.js" "${WEBUI_DIR}/styles/*.css" "${WEBUI_DIR}/assets/*.svg")
    add_custom_command(
        OUTPUT  "${WEBUI_BUILD}/bundle.js"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${WEBUI_BUILD}"
        COMMAND "${NPXCMD}" esbuild "${WEBUI_DIR}/main.jsx"
                    --bundle --conditions=source "--inject:${WEBUI_DIR}/react-globals.js"
                    --loader:.css=text --loader:.svg=dataurl
                    "--banner:js=/*! Bundles WEBMIDI.js (c) Jean-Philippe Cote, Apache-2.0, https://webmidijs.org - see @enkerli/webmidi/THIRD_PARTY_LICENSES.md */"
                    "--define:process.env.NODE_ENV=\"production\"" --minify
                    "--outfile=${WEBUI_BUILD}/bundle.js"
        DEPENDS ${WEBUI_SRC}
        WORKING_DIRECTORY "${WEBUI_DIR}"
        COMMENT "Bundling Serpe WebUI with esbuild"
        VERBATIM)
    add_custom_target(SerpeWebUIBundle DEPENDS "${WEBUI_BUILD}/bundle.js")
else()
    message(WARNING "npx not found — Serpe WebUI bundle.js must be pre-built at ${WEBUI_BUILD}/bundle.js")
endif()

juce_add_binary_data(SerpeWebUI
    HEADER_NAME BinaryDataWebUI.h
    NAMESPACE   BinaryData
    SOURCES
        "${WEBUI_DIR}/index.html"
        "${WEBUI_BUILD}/bundle.js")
if(NPXCMD)
    add_dependencies(SerpeWebUI SerpeWebUIBundle)
endif()
target_link_libraries(Serpe PRIVATE SerpeWebUI)

# PlatformSpecific.h requires exactly one of these (the .jucer set SERPE_DESKTOP).
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(Serpe PRIVATE SERPE_IPAD=1)

    # ── iPadOS-26 AUv3 icon (see Documentation/ICON_iPadOS26.md) ─────────────
    # On iPadOS 26 the appex icon must come from a COMPILED asset catalog with
    # Any/Dark/Tinted appearances. JUCE already generates a single-appearance
    # AppIcon.appiconset (from ICON_BIG) into each format target's JuceLibraryCode
    # Images.xcassets, and actool runs on THAT catalog — so we can't just add a
    # second catalog (actool then can't find our appicon, and merging two
    # catalogs is fragile). Instead, after juceaide has generated JUCE's catalog,
    # overwrite its AppIcon.appiconset with our appearance version (same name, so
    # the default ASSETCATALOG_COMPILER_APPICON_NAME=AppIcon still applies and
    # there is exactly ONE catalog with ONE AppIcon). Re-applied every configure,
    # so it survives JUCE regenerating the catalog. Source of truth:
    # Assets/Serpe.xcassets/SerpeAppIcon.appiconset.
    set(_serpe_appicon_src "${CMAKE_SOURCE_DIR}/Assets/Serpe.xcassets/SerpeAppIcon.appiconset")
    foreach(_fmt Serpe_AUv3 Serpe_Standalone)
        set(_juce_appiconset
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_artefacts/JuceLibraryCode/${_fmt}/Images.xcassets/AppIcon.appiconset")
        if(EXISTS "${_juce_appiconset}")
            file(REMOVE_RECURSE "${_juce_appiconset}")
            file(COPY "${_serpe_appicon_src}/" DESTINATION "${_juce_appiconset}")
            message(STATUS "Serpe: installed appearance AppIcon for ${_fmt}")
        else()
            message(WARNING "Serpe: ${_fmt} appiconset not found at ${_juce_appiconset}; "
                            "AUv3 appearance icon not installed (reconfigure after the first generate).")
        endif()
    endforeach()
else()
    target_compile_definitions(Serpe PRIVATE SERPE_DESKTOP=1)
endif()

target_link_libraries(Serpe
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Conformance test (re-homed from the .jucer post-build) ───────────────────
# Locks the plugin's numeric notation to the suite vectors. Encode/display side
# (PatternUtils); links juce_core only. `ctest` runs it; CMake also wires it as
# a post-build step on the plugin so a normal build surfaces drift.
#
# Host-only: it is a console app that runs on the build machine. On an iOS build
# the binary targets the device, so neither the POST_BUILD run nor ctest can
# execute it on the macOS host — guard the whole block to non-iOS builds.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_testing()
    juce_add_console_app(serpe_conformance PRODUCT_NAME "serpe_conformance")
    target_sources(serpe_conformance PRIVATE
        Source/Tests/RhythmConformanceTests.cpp
        Source/Core/PatternUtils.cpp)
    # The conformance app needs no networking; disable curl (juce_core defaults
    # it on for Linux and would otherwise need libcurl linked — undefined refs).
    target_compile_definitions(serpe_conformance PRIVATE JUCE_USE_CURL=0)
    juce_generate_juce_header(serpe_conformance)
    target_include_directories(serpe_conformance PRIVATE Source/Core Source/Tests)
    target_link_libraries(serpe_conformance
        PRIVATE
            juce::juce_core
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
    add_test(NAME rhythm_conformance COMMAND serpe_conformance)

    add_custom_command(TARGET Serpe POST_BUILD
        COMMAND serpe_conformance
        COMMENT "Rhythm codec conformance (suite-wide leftmost-LSB vectors)")

    # ── Poly-lane conformance (music-suite docs/SERPE_POLY.md §8 milestone 1) ──
    # Locks PolyParser (lane splitting, labels, offsets, lcm) to the same
    # vectors the webapp's parsePolyUPI is tested against. PolyParser calls
    # UPIParser::parse() per lane, so this app needs the full mono parser
    # and its dependencies, not just PatternUtils.
    juce_add_console_app(serpe_poly_conformance PRODUCT_NAME "serpe_poly_conformance")
    target_sources(serpe_poly_conformance PRIVATE
        Source/Tests/PolyConformanceTests.cpp
        Source/Core/PolyParser.cpp
        Source/Core/UPIParser.cpp
        Source/Core/PatternEngine.cpp
        Source/Core/PatternUtils.cpp
        Source/Core/QuantizationEngine.cpp)
    target_compile_definitions(serpe_poly_conformance PRIVATE JUCE_USE_CURL=0)
    juce_generate_juce_header(serpe_poly_conformance)
    target_include_directories(serpe_poly_conformance PRIVATE Source/Core Source/Tests)
    target_link_libraries(serpe_poly_conformance
        PRIVATE
            juce::juce_core
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
    add_test(NAME poly_conformance COMMAND serpe_poly_conformance)

    add_custom_command(TARGET Serpe POST_BUILD
        COMMAND serpe_poly_conformance
        COMMENT "Poly-lane conformance (lane splitting/offsets/lcm vs the webapp)")

endif()
