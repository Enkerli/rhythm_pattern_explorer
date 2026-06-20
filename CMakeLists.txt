cmake_minimum_required(VERSION 3.22)
project(Serpe VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# ── Suite foundation (enkerli-juce submodule): JUCE resolution + archetypes ──
include("${CMAKE_CURRENT_SOURCE_DIR}/enkerli-juce/cmake/EnkerliPlugin.cmake")
enkerli_resolve_juce()

# Force the iOS signing team project-wide. The archetype passes DEVELOPMENT_TEAM
# to juce_add_plugin, but CMake's Xcode generator still fills every target with
# the machine's *default* Apple account team (here B2623T4238) — so the project
# came up needing a manual switch to the intended team. Setting the Xcode
# attribute before any target is created applies it to all of them (SharedCode,
# Standalone, AUv3, install). Must precede enkerli_add_*_plugin.
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${ENKERLI_IOS_TEAM_ID}")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic")
endif()

# ── Plugin (midi_effect / aumi archetype) ───────────────────────────────────
# Identity is load-bearing: PLUGIN_CODE RPEd, manufacturer Enke, bundle
# com.enkerli.serpe — installed hosts key saved sessions on these. The
# archetype would lowercase the bundle id, so it is passed explicitly.
enkerli_add_midi_effect_plugin(Serpe
    PLUGIN_CODE RPEd
    BUNDLE_ID   "com.enkerli.serpe"
    VERSION     "${PROJECT_VERSION}"
    DESCRIPTION "Mathematical rhythm pattern generator and analyzer"
    ICON_BIG    "${CMAKE_SOURCE_DIR}/Assets/serpe-icon-1024.png"
)

# The existing sources include <JuceHeader.h> (Projucer style); generate it so
# they compile unchanged under CMake.
juce_generate_juce_header(Serpe)

add_compile_options($<$<CXX_COMPILER_ID:GNU,Clang>:-finput-charset=UTF-8>)

# ── Sources (the native PluginEditor stays for now; M4 swaps it for WebView) ──
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
    Source/Platform/PluginEditor.cpp
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
    Source/Platform/PluginEditor.h
    Source/Platform/PlatformSpecific.h
    Source/Platform/DebugConfig.h
)

target_include_directories(Serpe PRIVATE
    Source Source/Core Source/Managers Source/Platform)

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
