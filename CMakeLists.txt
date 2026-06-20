cmake_minimum_required(VERSION 3.22)
project(Serpe VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# ── Suite foundation (enkerli-juce submodule): JUCE resolution + archetypes ──
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
    # Full-bleed master (the OS applies the corner mask); see Assets/icon and
    # DESIGN_HANDOFF §7. NOTE: the iPadOS-26 AUv3 icon also needs a per-appex
    # asset catalog with Any/Dark/Tinted appearances — see Documentation.
    ICON_BIG    "${CMAKE_SOURCE_DIR}/Assets/icon/serpe-1024-bleed.png"
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
set(WEBUI_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/Source/WebUI")
set(WEBUI_DIST "${WEBUI_DIR}/dist")

find_program(NPXCMD npx)
if(NPXCMD)
    file(GLOB_RECURSE WEBUI_SRC
        "${WEBUI_DIR}/main.jsx" "${WEBUI_DIR}/*.js"
        "${WEBUI_DIR}/engine/*.js" "${WEBUI_DIR}/styles/*.css" "${WEBUI_DIR}/index.html")
    add_custom_command(
        OUTPUT  "${WEBUI_DIST}/bundle.js" "${WEBUI_DIST}/bundle.css" "${WEBUI_DIST}/index.html"
        COMMAND "${NPXCMD}" esbuild "${WEBUI_DIR}/main.jsx"
                    --bundle "--inject:${WEBUI_DIR}/react-globals.js"
                    "--define:process.env.NODE_ENV=\"production\"" --minify
                    "--outfile=${WEBUI_DIST}/bundle.js"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${WEBUI_DIR}/index.html" "${WEBUI_DIST}/index.html"
        DEPENDS ${WEBUI_SRC}
        WORKING_DIRECTORY "${WEBUI_DIR}"
        COMMENT "Bundling Serpe WebUI with esbuild"
        VERBATIM)
    add_custom_target(SerpeWebUIBundle DEPENDS "${WEBUI_DIST}/bundle.js")
endif()

juce_add_binary_data(SerpeWebUI
    HEADER_NAME BinaryDataWebUI.h
    NAMESPACE   BinaryData
    SOURCES
        "${WEBUI_DIST}/index.html"
        "${WEBUI_DIST}/bundle.js"
        "${WEBUI_DIST}/bundle.css")
if(NPXCMD)
    add_dependencies(SerpeWebUI SerpeWebUIBundle)
endif()
target_link_libraries(Serpe PRIVATE SerpeWebUI)

# PlatformSpecific.h requires exactly one of these (the .jucer set SERPE_DESKTOP).
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(Serpe PRIVATE SERPE_IPAD=1)
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
        COMMENT "Rhythm codec conformance (suite-wide MSB-first vectors)")
endif()
