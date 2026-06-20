/*
  ==============================================================================

    PresetManager.h
    Preset system for Rhythm Pattern Explorer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Preset data structure containing complete plugin state
 */
struct PresetData
{
    juce::String name;
    juce::String category;
    juce::String description;
    juce::String upiPattern;
    juce::Time dateCreated;
    juce::Time dateModified;
    
    // Complete plugin state
    juce::ValueTree pluginState;
    
    // Quick access fields (extracted from pluginState for UI)
    juce::String backgroundColorName;
    bool hasScenes = false;
    bool hasProgressiveTransforms = false;
    bool hasAccentPattern = false;
    
    PresetData() : dateCreated(juce::Time::getCurrentTime()), dateModified(juce::Time::getCurrentTime()) {}
};

//==============================================================================
/**
 * Simple preset manager - foundation for future expansion
 * 
 * This is a minimal implementation that builds on existing state persistence.
 * Future phases can add advanced features like categories, search, etc.
 */
class PresetManager
{
public:
    PresetManager();
    ~PresetManager();
    
    //==============================================================================
    // Core preset operations
    bool savePreset(const juce::String& name, const juce::String& category, 
                   const juce::String& description, const juce::ValueTree& pluginState,
                   const juce::String& upiPattern);
    
    bool loadPreset(const juce::String& name, juce::ValueTree& pluginState);
    bool deletePreset(const juce::String& name);
    bool renamePreset(const juce::String& oldName, const juce::String& newName);
    
    //==============================================================================
    // Preset discovery and management
    juce::StringArray getPresetNames() const;
    juce::StringArray getCategories() const;
    juce::StringArray getPresetsInCategory(const juce::String& category) const;
    
    bool presetExists(const juce::String& name) const;
    PresetData getPresetData(const juce::String& name) const;
    
    //==============================================================================
    // File system operations
    juce::File getPresetDirectory() const;
    juce::File getPresetFile(const juce::String& name) const;
    void createPresetDirectoryIfNeeded();
    
    //==============================================================================
    // Factory presets
    void installFactoryPresets();
    bool isFactoryPreset(const juce::String& name) const;
    
    //==============================================================================
    // Validation and utilities
    bool isValidPresetName(const juce::String& name) const;
    juce::String sanitizePresetName(const juce::String& name) const;
    
private:
    //==============================================================================
    // Internal data
    std::map<juce::String, PresetData> cachedPresets;
    juce::StringArray factoryPresetNames;
    bool cacheValid = false;
    
    //==============================================================================
    // Internal methods
    void refreshCache();
    void addToCache(const PresetData& preset);
    bool loadPresetFromFile(const juce::File& file, PresetData& preset);
    bool savePresetToFile(const PresetData& preset);
    
    juce::ValueTree createFactoryPreset(const juce::String& name, 
                                       const juce::String& category,
                                       const juce::String& description, 
                                       const juce::String& upiPattern) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};