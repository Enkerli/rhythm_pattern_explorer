#pragma once

//==============================================================================
// Platform-specific defines and configurations for Serpe
// This file handles compile-time platform differences between:
// - Desktop macOS (AU/VST3): SERPE_DESKTOP defined
// - iPad iOS (AUv3): SERPE_IPAD defined
//==============================================================================

// Platform detection
#if defined(SERPE_DESKTOP)
    #define SERPE_IS_DESKTOP 1
    #define SERPE_IS_IPAD 0
    #define SERPE_PLATFORM_NAME "Desktop"
#elif defined(SERPE_IPAD)
    #define SERPE_IS_DESKTOP 0
    #define SERPE_IS_IPAD 1
    #define SERPE_PLATFORM_NAME "iPad"
#else
    #error "No platform defined. Define either SERPE_DESKTOP or SERPE_IPAD"
#endif

//==============================================================================
// Platform-specific feature toggles
//==============================================================================

// WebView Documentation System
#if SERPE_IS_DESKTOP
    #define SERPE_ENABLE_WEBVIEW 1
    #define SERPE_WEBVIEW_REASON "Full WebView support on macOS"
#else
    #define SERPE_ENABLE_WEBVIEW 0
    #define SERPE_WEBVIEW_REASON "Disabled on iOS due to GPU process conflicts"
#endif

// File System Access Level
#if SERPE_IS_DESKTOP
    #define SERPE_UNRESTRICTED_FILE_ACCESS 1
    #define SERPE_AUTO_CREATE_FACTORY_PRESETS 1
#else
    #define SERPE_UNRESTRICTED_FILE_ACCESS 0
    #define SERPE_AUTO_CREATE_FACTORY_PRESETS 0
#endif

// MIDI Note Duration Safety
#if SERPE_IS_DESKTOP
    #define SERPE_SINGLE_NOTE_OFF 1
    #define SERPE_REDUNDANT_NOTE_OFF 0
#else
    #define SERPE_SINGLE_NOTE_OFF 0
    #define SERPE_REDUNDANT_NOTE_OFF 1
#endif

// Debug Assertions (Critical for iOS)
#if SERPE_IS_IPAD
    // iOS requires assertions disabled due to Unicode validation conflicts
    #ifndef JUCE_DISABLE_ASSERTIONS
        #define JUCE_DISABLE_ASSERTIONS 1
    #endif
#endif

//==============================================================================
// Platform-specific plugin formats
//==============================================================================

#if SERPE_IS_DESKTOP
    #define SERPE_SUPPORTS_AU 1
    #define SERPE_SUPPORTS_VST3 1  
    #define SERPE_SUPPORTS_AUV3 0
#else
    #define SERPE_SUPPORTS_AU 0
    #define SERPE_SUPPORTS_VST3 0
    #define SERPE_SUPPORTS_AUV3 1
#endif

//==============================================================================
// Platform-specific constants
//==============================================================================

namespace SerpeConstants
{
    // Note timing safety margins
    #if SERPE_IS_DESKTOP
        constexpr int primaryNoteOffDelay = 1;    // Standard 1-tick delay
        constexpr int safetyNoteOffDelay = 0;     // No redundant note-off needed
    #else
        constexpr int primaryNoteOffDelay = 1;    // Primary note-off at +1
        constexpr int safetyNoteOffDelay = 10;    // Safety note-off at +10
    #endif
    
    // Preset directory behavior
    #if SERPE_IS_DESKTOP
        constexpr bool autoCreatePresetDirectories = true;
        constexpr bool requireUserInteractionForPresets = false;
    #else
        constexpr bool autoCreatePresetDirectories = false;
        constexpr bool requireUserInteractionForPresets = true;
    #endif
}

//==============================================================================
// Compile-time platform information
//==============================================================================

namespace SerpePlatform
{
    constexpr bool isDesktop() { return SERPE_IS_DESKTOP; }
    constexpr bool isIPad() { return SERPE_IS_IPAD; }
    constexpr const char* getName() { return SERPE_PLATFORM_NAME; }
    
    constexpr bool hasWebView() { return SERPE_ENABLE_WEBVIEW; }
    constexpr bool hasUnrestrictedFileAccess() { return SERPE_UNRESTRICTED_FILE_ACCESS; }
    constexpr bool needsRedundantNoteOff() { return SERPE_REDUNDANT_NOTE_OFF; }
}