/*

    IMPORTANT! This file is auto-generated each time you save your
    project - if you alter its contents, your changes may be overwritten!

*/

#pragma once

//==============================================================================
// Audio plugin settings..

#ifndef  JucePlugin_Build_VST
 #define JucePlugin_Build_VST              0
#endif
#ifndef  JucePlugin_Build_VST3
 #define JucePlugin_Build_VST3             1
#endif
#ifndef  JucePlugin_Build_AU
 #define JucePlugin_Build_AU               1
#endif
#ifndef  JucePlugin_Build_AUv3
 #define JucePlugin_Build_AUv3             0
#endif
#ifndef  JucePlugin_Build_AAX
 #define JucePlugin_Build_AAX              0
#endif
#ifndef  JucePlugin_Build_Standalone
 #define JucePlugin_Build_Standalone       0
#endif
#ifndef  JucePlugin_Build_Unity
 #define JucePlugin_Build_Unity            0
#endif
#ifndef  JucePlugin_Build_LV2
 #define JucePlugin_Build_LV2              0
#endif
#ifndef  JucePlugin_Enable_IAA
 #define JucePlugin_Enable_IAA             0
#endif
#ifndef  JucePlugin_Enable_ARA
 #define JucePlugin_Enable_ARA             0
#endif
#ifndef  JucePlugin_Name
 #define JucePlugin_Name                   "Rhythm Pattern Explorer"
#endif
#ifndef  JucePlugin_Desc
 #define JucePlugin_Desc                   "Mathematical rhythm pattern generator and analyzer"
#endif
#ifndef  JucePlugin_Manufacturer
 #define JucePlugin_Manufacturer           "Rhythm Pattern Explorer"
#endif
#ifndef  JucePlugin_ManufacturerWebsite
 #define JucePlugin_ManufacturerWebsite    "https://github.com/enkerli/rhythm_pattern_explorer"
#endif
#ifndef  JucePlugin_ManufacturerEmail
 #define JucePlugin_ManufacturerEmail      "support@rhythmpatternexplorer.com"
#endif
#ifndef  JucePlugin_ManufacturerCode
 #define JucePlugin_ManufacturerCode       0x52685074
#endif
#ifndef  JucePlugin_PluginCode
 #define JucePlugin_PluginCode             0x52685078
#endif
#ifndef  JucePlugin_IsSynth
 #define JucePlugin_IsSynth                0
#endif
#ifndef  JucePlugin_WantsMidiInput
 #define JucePlugin_WantsMidiInput         1
#endif
#ifndef  JucePlugin_ProducesMidiOutput
 #define JucePlugin_ProducesMidiOutput     1
#endif
#ifndef  JucePlugin_IsMidiEffect
 #define JucePlugin_IsMidiEffect           1
#endif
#ifndef  JucePlugin_EditorRequiresKeyboardFocus
 #define JucePlugin_EditorRequiresKeyboardFocus  0
#endif
#ifndef  JucePlugin_Version
 #define JucePlugin_Version                1.3.5
#endif
#ifndef  JucePlugin_VersionCode
 #define JucePlugin_VersionCode            0x10305
#endif
#ifndef  JucePlugin_VersionString
 #define JucePlugin_VersionString          "1.3.5"
#endif
#ifndef  JucePlugin_VSTUniqueID
 #define JucePlugin_VSTUniqueID            JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_VSTCategory
 #define JucePlugin_VSTCategory            kPlugCategEffect
#endif
#ifndef  JucePlugin_Vst3Category
 #define JucePlugin_Vst3Category           "Fx"
#endif
#ifndef  JucePlugin_AUMainType
 #define JucePlugin_AUMainType             'aumi'
#endif
#ifndef  JucePlugin_AUSubType
 #define JucePlugin_AUSubType              JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_AUExportPrefix
 #define JucePlugin_AUExportPrefix         RhythmPatternExplorerAU
#endif
#ifndef  JucePlugin_AUExportPrefixQuoted
 #define JucePlugin_AUExportPrefixQuoted   "RhythmPatternExplorerAU"
#endif
#ifndef  JucePlugin_AUManufacturerCode
 #define JucePlugin_AUManufacturerCode     JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_CFBundleIdentifier
 #define JucePlugin_CFBundleIdentifier     com.rhythmpatternexplorer.auv3
#endif
#ifndef  JucePlugin_AAXIdentifier
 #define JucePlugin_AAXIdentifier          com.rhythmpatternexplorer.auv3
#endif
#ifndef  JucePlugin_AAXManufacturerCode
 #define JucePlugin_AAXManufacturerCode    JucePlugin_ManufacturerCode
#endif
#ifndef  JucePlugin_AAXProductId
 #define JucePlugin_AAXProductId           JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_AAXCategory
 #define JucePlugin_AAXCategory            2
#endif
#ifndef  JucePlugin_AAXDisableBypass
 #define JucePlugin_AAXDisableBypass       0
#endif
#ifndef  JucePlugin_AAXDisableMultiMono
 #define JucePlugin_AAXDisableMultiMono    0
#endif
#ifndef  JucePlugin_IAAType
 #define JucePlugin_IAAType                0x6175726d
#endif
#ifndef  JucePlugin_IAASubType
 #define JucePlugin_IAASubType             JucePlugin_PluginCode
#endif
#ifndef  JucePlugin_IAAName
 #define JucePlugin_IAAName                "Rhythm Pattern Explorer: Rhythm Pattern Explorer"
#endif
#ifndef  JucePlugin_VSTNumMidiInputs
 #define JucePlugin_VSTNumMidiInputs       16
#endif
#ifndef  JucePlugin_VSTNumMidiOutputs
 #define JucePlugin_VSTNumMidiOutputs      16
#endif
#ifndef  JucePlugin_ARAContentTypes
 #define JucePlugin_ARAContentTypes        0
#endif
#ifndef  JucePlugin_ARATransformationFlags
 #define JucePlugin_ARATransformationFlags  0
#endif
#ifndef  JucePlugin_ARAFactoryID
 #define JucePlugin_ARAFactoryID           "com.RhythmPatternExplorer.RhythmPatternExplorer.factory"
#endif
#ifndef  JucePlugin_ARADocumentArchiveID
 #define JucePlugin_ARADocumentArchiveID   "com.RhythmPatternExplorer.RhythmPatternExplorer.aradocumentarchive.1.3.5"
#endif
#ifndef  JucePlugin_ARACompatibleArchiveIDs
 #define JucePlugin_ARACompatibleArchiveIDs  ""
#endif
#ifndef  JucePlugin_MaxNumInputChannels
 #define JucePlugin_MaxNumInputChannels    0
#endif
#ifndef  JucePlugin_MaxNumOutputChannels
 #define JucePlugin_MaxNumOutputChannels   0
#endif
#ifndef  JucePlugin_PreferredChannelConfigurations
 #define JucePlugin_PreferredChannelConfigurations  {0,0}
#endif
