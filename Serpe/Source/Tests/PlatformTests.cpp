#include "PlatformTests.h"

bool PlatformTests::runAllTests()
{
    bool allPassed = true;
    
    allPassed &= testPlatformDetection();
    allPassed &= testFeatureToggles();
    allPassed &= testMidiHandling();
    
    return allPassed;
}

bool PlatformTests::testPlatformDetection()
{
    // Test platform detection macros
    #if SERPE_IS_DESKTOP
        // Desktop platform tests
        return SERPE_ENABLE_WEBVIEW && SERPE_UNRESTRICTED_FILE_ACCESS;
    #elif SERPE_IS_IPAD
        // iPad platform tests  
        return !SERPE_ENABLE_WEBVIEW && !SERPE_UNRESTRICTED_FILE_ACCESS;
    #else
        return false; // No platform defined
    #endif
}

bool PlatformTests::testFeatureToggles()
{
    // Test feature toggle consistency
    return (SerpePlatform::hasWebView() == SERPE_ENABLE_WEBVIEW) &&
           (SerpePlatform::hasUnrestrictedFileAccess() == SERPE_UNRESTRICTED_FILE_ACCESS) &&
           (SerpePlatform::needsRedundantNoteOff() == SERPE_REDUNDANT_NOTE_OFF);
}

bool PlatformTests::testMidiHandling()
{
    // Test platform-specific MIDI constants
    #if SERPE_IS_DESKTOP
        return SerpeConstants::primaryNoteOffDelay == 1 &&
               SerpeConstants::safetyNoteOffDelay == 0;
    #else
        return SerpeConstants::primaryNoteOffDelay == 1 &&
               SerpeConstants::safetyNoteOffDelay == 10;
    #endif
}