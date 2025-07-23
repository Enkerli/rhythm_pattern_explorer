/*
  ==============================================================================

    Debug Configuration - Centralized debug logging controls
    
    Provides compile-time debug controls with category-based filtering
    for better performance in release builds.

  ==============================================================================
*/

#pragma once

// Debug build detection
#ifdef DEBUG
    #define RPE_DEBUG_BUILD 1
#else
    #define RPE_DEBUG_BUILD 0
#endif

// Debug categories - can be individually enabled/disabled
#define RPE_DEBUG_TRANSPORT     (RPE_DEBUG_BUILD && 0)  // Transport timing
#define RPE_DEBUG_PATTERNS      (RPE_DEBUG_BUILD && 1)  // Pattern parsing/generation
#define RPE_DEBUG_PROGRESSIVE   (RPE_DEBUG_BUILD && 1)  // Progressive transformations
#define RPE_DEBUG_SCENES        (RPE_DEBUG_BUILD && 1)  // Scene management
#define RPE_DEBUG_ACCENTS       (RPE_DEBUG_BUILD && 1)  // Accent patterns
#define RPE_DEBUG_GENERAL       (RPE_DEBUG_BUILD && 1)  // General plugin operations

// Centralized debug macros
#if RPE_DEBUG_BUILD
    #include <JuceHeader.h>
    #define RPE_DBG_TRANSPORT(text) do { if (RPE_DEBUG_TRANSPORT) { DBG("[TRANSPORT] " << text); } } while(0)
    #define RPE_DBG_PATTERNS(text)  do { if (RPE_DEBUG_PATTERNS)  { DBG("[PATTERNS] " << text); } } while(0)
    #define RPE_DBG_PROGRESSIVE(text) do { if (RPE_DEBUG_PROGRESSIVE) { DBG("[PROGRESSIVE] " << text); } } while(0)
    #define RPE_DBG_SCENES(text)    do { if (RPE_DEBUG_SCENES)    { DBG("[SCENES] " << text); } } while(0)
    #define RPE_DBG_ACCENTS(text)   do { if (RPE_DEBUG_ACCENTS)   { DBG("[ACCENTS] " << text); } } while(0)
    #define RPE_DBG_GENERAL(text)   do { if (RPE_DEBUG_GENERAL)   { DBG("[GENERAL] " << text); } } while(0)
#else
    // Release builds - all debug output compiled out
    #define RPE_DBG_TRANSPORT(text) do { } while(0)
    #define RPE_DBG_PATTERNS(text)  do { } while(0)
    #define RPE_DBG_PROGRESSIVE(text) do { } while(0)
    #define RPE_DBG_SCENES(text)    do { } while(0)
    #define RPE_DBG_ACCENTS(text)   do { } while(0)
    #define RPE_DBG_GENERAL(text)   do { } while(0)
#endif

// Legacy DBG macro compatibility - disable in release builds
#if !RPE_DEBUG_BUILD
    #ifdef DBG
        #undef DBG
    #endif
    #define DBG(text) do { } while(0)
#endif