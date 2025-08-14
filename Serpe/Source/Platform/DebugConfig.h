/*
  ==============================================================================

    Serpe Debug Configuration - Centralized debug logging controls
    
    Provides compile-time debug controls with category-based filtering
    for better performance in release builds.

  ==============================================================================
*/

#pragma once

// Debug build detection
#ifdef DEBUG
    #define SERPE_DEBUG_BUILD 1
#else
    #define SERPE_DEBUG_BUILD 0
#endif

// Debug categories - can be individually enabled/disabled
#define SERPE_DEBUG_TRANSPORT     (SERPE_DEBUG_BUILD && 0)  // Transport timing
#define SERPE_DEBUG_PATTERNS      (SERPE_DEBUG_BUILD && 1)  // Pattern parsing/generation
#define SERPE_DEBUG_PROGRESSIVE   (SERPE_DEBUG_BUILD && 1)  // Progressive transformations
#define SERPE_DEBUG_SCENES        (SERPE_DEBUG_BUILD && 1)  // Scene management
#define SERPE_DEBUG_GENERAL       (SERPE_DEBUG_BUILD && 1)  // General plugin operations

// Centralized debug macros
#if SERPE_DEBUG_BUILD
    #include <JuceHeader.h>
    #define SERPE_DBG_TRANSPORT(text) do { if (SERPE_DEBUG_TRANSPORT) { DBG("[TRANSPORT] " << text); } } while(0)
    #define SERPE_DBG_PATTERNS(text)  do { if (SERPE_DEBUG_PATTERNS)  { DBG("[PATTERNS] " << text); } } while(0)
    #define SERPE_DBG_PROGRESSIVE(text) do { if (SERPE_DEBUG_PROGRESSIVE) { DBG("[PROGRESSIVE] " << text); } } while(0)
    #define SERPE_DBG_SCENES(text)    do { if (SERPE_DEBUG_SCENES)    { DBG("[SCENES] " << text); } } while(0)
    #define SERPE_DBG_GENERAL(text)   do { if (SERPE_DEBUG_GENERAL)   { DBG("[GENERAL] " << text); } } while(0)
#else
    // Release builds - all debug output compiled out
    #define SERPE_DBG_TRANSPORT(text) do { } while(0)
    #define SERPE_DBG_PATTERNS(text)  do { } while(0)
    #define SERPE_DBG_PROGRESSIVE(text) do { } while(0)
    #define SERPE_DBG_SCENES(text)    do { } while(0)
    #define SERPE_DBG_GENERAL(text)   do { } while(0)
#endif

// Legacy DBG macro compatibility - disable in release builds  
#if !SERPE_DEBUG_BUILD
    #ifdef DBG
        #undef DBG
    #endif
    #define DBG(text) do { } while(0)
#endif