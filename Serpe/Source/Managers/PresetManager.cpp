/*
  ==============================================================================

    PresetManager.cpp
    Preset system implementation for Rhythm Pattern Explorer
    
    Provides comprehensive preset management with:
    - Factory preset installation with rich pattern examples
    - User preset creation, saving, loading, and deletion
    - Automatic feature detection (scenes, progressive transforms, accents)
    - JSON-based preset storage with embedded plugin state
    - Cached preset management for performance
    - Safe file operations with validation and sanitization

  ==============================================================================
*/

#include "PresetManager.h"

//==============================================================================
/**
 * Initialize preset manager and load existing presets
 * Creates preset directory if needed and populates cache for fast access
 */
PresetManager::PresetManager()
{
    createPresetDirectoryIfNeeded();
    refreshCache();
}

PresetManager::~PresetManager() = default;

//==============================================================================
/**
 * Save a preset with comprehensive metadata extraction
 * 
 * Automatically detects pattern features:
 * - Scenes: patterns containing '|' separator
 * - Progressive transforms: patterns with '>', '+', or '*' operators
 * - Accent patterns: patterns with '{...}' notation
 * 
 * @param name Preset display name (will be sanitized for filesystem)
 * @param category Organization category (defaults to "User")
 * @param description Human-readable description
 * @param pluginState Complete plugin state tree for restoration
 * @param upiPattern UPI pattern string for feature analysis
 * @return true if preset was successfully saved
 */
bool PresetManager::savePreset(const juce::String& name, const juce::String& category, 
                               const juce::String& description, const juce::ValueTree& pluginState,
                               const juce::String& upiPattern)
{
    if (!isValidPresetName(name))
        return false;
    
    PresetData preset;
    preset.name = sanitizePresetName(name);
    preset.category = category.isEmpty() ? "User" : category;
    preset.description = description;
    preset.upiPattern = upiPattern;
    preset.pluginState = pluginState.createCopy();
    preset.dateModified = juce::Time::getCurrentTime();
    
    // Extract quick access metadata from plugin state for UI display
    preset.backgroundColorName = pluginState.getProperty("currentBackgroundColor", "Dark").toString();
    
    // Feature detection for preset browser icons and functionality
    preset.hasScenes = upiPattern.contains("|");  // Scene separation with pipe
    preset.hasProgressiveTransforms = upiPattern.contains(">") || upiPattern.contains("%") || upiPattern.contains("+") || upiPattern.contains("*");  // Various progressive operators
    preset.hasAccentPattern = upiPattern.contains("{") && upiPattern.contains("}");  // Accent pattern notation
    
    // If this is a new preset, set creation time
    if (!presetExists(preset.name))
        preset.dateCreated = preset.dateModified;
    else
        preset.dateCreated = getPresetData(preset.name).dateCreated;
    
    if (savePresetToFile(preset))
    {
        addToCache(preset);
        return true;
    }
    
    return false;
}

bool PresetManager::loadPreset(const juce::String& name, juce::ValueTree& pluginState)
{
    if (!presetExists(name))
        return false;
    
    const auto preset = getPresetData(name);
    if (preset.pluginState.isValid())
    {
        pluginState = preset.pluginState.createCopy();
        return true;
    }
    
    return false;
}

bool PresetManager::deletePreset(const juce::String& name)
{
    if (!presetExists(name) || isFactoryPreset(name))
        return false;
    
    auto file = getPresetFile(name);
    if (file.deleteFile())
    {
        cachedPresets.erase(name);
        return true;
    }
    
    return false;
}

bool PresetManager::renamePreset(const juce::String& oldName, const juce::String& newName)
{
    if (!presetExists(oldName) || isFactoryPreset(oldName) || !isValidPresetName(newName))
        return false;
    
    auto preset = getPresetData(oldName);
    preset.name = sanitizePresetName(newName);
    preset.dateModified = juce::Time::getCurrentTime();
    
    // Save with new name and delete old
    if (savePresetToFile(preset))
    {
        deletePreset(oldName);
        addToCache(preset);
        return true;
    }
    
    return false;
}

//==============================================================================
juce::StringArray PresetManager::getPresetNames() const
{
    if (!cacheValid)
        const_cast<PresetManager*>(this)->refreshCache();
    
    juce::StringArray names;
    for (const auto& [name, preset] : cachedPresets)
        names.add(name);
    
    names.sort(true); // Case-insensitive sort
    return names;
}

juce::StringArray PresetManager::getCategories() const
{
    if (!cacheValid)
        const_cast<PresetManager*>(this)->refreshCache();
    
    juce::StringArray categories;
    for (const auto& [name, preset] : cachedPresets)
    {
        if (!categories.contains(preset.category))
            categories.add(preset.category);
    }
    
    categories.sort(true);
    return categories;
}

juce::StringArray PresetManager::getPresetsInCategory(const juce::String& category) const
{
    if (!cacheValid)
        const_cast<PresetManager*>(this)->refreshCache();
    
    juce::StringArray names;
    for (const auto& [name, preset] : cachedPresets)
    {
        if (preset.category == category)
            names.add(name);
    }
    
    names.sort(true);
    return names;
}

bool PresetManager::presetExists(const juce::String& name) const
{
    if (!cacheValid)
        const_cast<PresetManager*>(this)->refreshCache();
    
    return cachedPresets.find(name) != cachedPresets.end();
}

PresetData PresetManager::getPresetData(const juce::String& name) const
{
    if (!cacheValid)
        const_cast<PresetManager*>(this)->refreshCache();
    
    auto it = cachedPresets.find(name);
    return (it != cachedPresets.end()) ? it->second : PresetData{};
}

//==============================================================================
juce::File PresetManager::getPresetDirectory() const
{
    // iOS apps must use userApplicationDataDirectory due to sandboxing
    #if SERPE_IS_IPAD
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("RhythmPatternExplorer")
               .getChildFile("Presets");
    #else
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
               .getChildFile("RhythmPatternExplorer")
               .getChildFile("Presets");
    #endif
}

juce::File PresetManager::getPresetFile(const juce::String& name) const
{
    return getPresetDirectory().getChildFile(sanitizePresetName(name) + ".rpepreset");
}

void PresetManager::createPresetDirectoryIfNeeded()
{
    auto dir = getPresetDirectory();
    if (!dir.exists())
        dir.createDirectory();
}

//==============================================================================
/**
 * Install comprehensive factory preset library
 * 
 * Provides example presets covering all major pattern types:
 * - Basic Patterns: Classic rhythms (tresillo, clave, cinquillo)
 * - Progressive: Growing/evolving patterns with transformations
 * - Accent Patterns: Suprasegmental accent layer examples
 * - Advanced: Complex combinations showcasing multiple features
 * 
 * Factory presets are protected from deletion and serve as learning examples
 */
void PresetManager::installFactoryPresets()
{
    // Install comprehensive factory preset library with educational examples
    struct FactoryPreset {
        juce::String name, category, description, upi;
    };
    
    const FactoryPreset factoryPresets[] = {
        // Basic Patterns - Educational rhythmic fundamentals
        {"Tresillo Classic", "Basic Patterns", "Classic 3-against-8 Afro-Cuban tresillo pattern", "E(3,8)"},
        {"Son Clave", "Basic Patterns", "2-3 son clave pattern using scenes", "E(3,8)|E(2,8)"},
        {"Cinquillo", "Basic Patterns", "Five-note Cuban pattern", "E(5,8)"},
        {"Bossa Nova", "Basic Patterns", "Bossa nova-style pattern", "E(3,8)"},
        
        // Progressive Patterns - Dynamic evolution examples
        {"Tresillo Growth", "Progressive", "Tresillo growing to full quintillo", "E(3,8)>5"},
        {"Euclidean Evolution", "Progressive", "Single onset evolving to complex pattern", "E(1,16)>8"},
        {"Rotating Rhythm", "Progressive", "Tresillo with progressive rotation", "E(3,8)+1"},
        
        // Accent Patterns - Suprasegmental accent layer examples
        {"Accented Tresillo", "Accent Patterns", "Tresillo with accent on first onset", "{100}E(3,8)"},
        {"Polyrhythmic Accents", "Accent Patterns", "Complex polyrhythmic accent pattern", "{10010}E(5,8)"},
        {"Binary Accents", "Accent Patterns", "Simple alternating accents", "{10}E(4,8)"},
        
        // Complex Combinations - Advanced multi-feature demonstrations
        {"Progressive Scenes", "Advanced", "Scene cycling with progressive transforms", "E(3,8)>5|E(5,13)|B(7,16)"},
        {"Accented Evolution", "Advanced", "Progressive pattern with accent layer", "{101}E(1,8)>8"},
        {"Barlow Transformation", "Advanced", "Barlow indispensability progression", "E(3,8)B>8"}
    };
    
    for (const auto& fp : factoryPresets)
    {
        if (!presetExists(fp.name))
        {
            auto factoryState = createFactoryPreset(fp.name, fp.category, fp.description, fp.upi);
            savePreset(fp.name, fp.category, fp.description, factoryState, fp.upi);
            factoryPresetNames.add(fp.name);
        }
    }
}

bool PresetManager::isFactoryPreset(const juce::String& name) const
{
    return factoryPresetNames.contains(name);
}

//==============================================================================
bool PresetManager::isValidPresetName(const juce::String& name) const
{
    if (name.isEmpty() || name.length() > 64)
        return false;
    
    // Check for invalid filename characters
    const juce::String invalidChars = "<>:\"/\\|?*";
    for (int i = 0; i < invalidChars.length(); ++i)
    {
        if (name.containsChar(invalidChars[i]))
            return false;
    }
    
    return true;
}

juce::String PresetManager::sanitizePresetName(const juce::String& name) const
{
    juce::String sanitized = name.trim();
    
    // Replace invalid characters with underscores
    const juce::String invalidChars = "<>:\"/\\|?*";
    for (int i = 0; i < invalidChars.length(); ++i)
    {
        sanitized = sanitized.replace(juce::String::charToString(invalidChars[i]), "_");
    }
    
    return sanitized.substring(0, 64); // Limit length
}

//==============================================================================
void PresetManager::refreshCache()
{
    cachedPresets.clear();
    
    // Install factory presets on first access if not already installed
    // This provides iPad compatibility by avoiding file I/O during initialization
    #if SERPE_IS_IPAD
    static bool factoryPresetsInstalled = false;
    if (!factoryPresetsInstalled) {
        try {
            installFactoryPresets();
            factoryPresetsInstalled = true;
        } catch (...) {
            // If factory preset installation fails, continue with user presets
        }
    }
    #endif
    
    auto presetDir = getPresetDirectory();
    if (!presetDir.exists())
    {
        cacheValid = true;
        return;
    }
    
    auto files = presetDir.findChildFiles(juce::File::findFiles, false, "*.rpepreset");
    
    for (const auto& file : files)
    {
        PresetData preset;
        if (loadPresetFromFile(file, preset))
        {
            cachedPresets[preset.name] = preset;
        }
    }
    
    cacheValid = true;
}

void PresetManager::addToCache(const PresetData& preset)
{
    cachedPresets[preset.name] = preset;
}

bool PresetManager::loadPresetFromFile(const juce::File& file, PresetData& preset)
{
    if (!file.exists())
        return false;
    
    auto json = juce::JSON::parse(file);
    if (!json.isObject())
        return false;
    
    auto obj = json.getDynamicObject();
    if (!obj)
        return false;
    
    preset.name = obj->getProperty("name").toString();
    preset.category = obj->getProperty("category").toString();
    preset.description = obj->getProperty("description").toString();
    preset.upiPattern = obj->getProperty("upiPattern").toString();
    preset.dateCreated = juce::Time(static_cast<int64_t>(obj->getProperty("dateCreated")));
    preset.dateModified = juce::Time(static_cast<int64_t>(obj->getProperty("dateModified")));
    
    // Load plugin state from embedded XML
    auto stateXmlString = obj->getProperty("pluginState").toString();
    if (auto xml = juce::parseXML(stateXmlString))
    {
        preset.pluginState = juce::ValueTree::fromXml(*xml);
    }
    
    // Extract quick access metadata
    preset.backgroundColorName = obj->getProperty("backgroundColorName").toString();
    preset.hasScenes = obj->getProperty("hasScenes");
    preset.hasProgressiveTransforms = obj->getProperty("hasProgressiveTransforms");
    preset.hasAccentPattern = obj->getProperty("hasAccentPattern");
    
    return preset.pluginState.isValid();
}

bool PresetManager::savePresetToFile(const PresetData& preset)
{
    auto file = getPresetFile(preset.name);
    
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("name", preset.name);
    obj->setProperty("category", preset.category);
    obj->setProperty("description", preset.description);
    obj->setProperty("upiPattern", preset.upiPattern);
    obj->setProperty("dateCreated", static_cast<int64_t>(preset.dateCreated.toMilliseconds()));
    obj->setProperty("dateModified", static_cast<int64_t>(preset.dateModified.toMilliseconds()));
    
    // Save plugin state as embedded XML
    if (auto xml = preset.pluginState.createXml())
    {
        obj->setProperty("pluginState", xml->toString());
    }
    
    // Save quick access metadata
    obj->setProperty("backgroundColorName", preset.backgroundColorName);
    obj->setProperty("hasScenes", preset.hasScenes);
    obj->setProperty("hasProgressiveTransforms", preset.hasProgressiveTransforms);
    obj->setProperty("hasAccentPattern", preset.hasAccentPattern);
    
    juce::var jsonData(obj.get());
    auto jsonString = juce::JSON::toString(jsonData, true);
    
    return file.replaceWithText(jsonString);
}

juce::ValueTree PresetManager::createFactoryPreset(const juce::String& name, 
                                                  const juce::String& category,
                                                  const juce::String& description, 
                                                  const juce::String& upiPattern) const
{
    // Create a minimal but valid plugin state for factory presets
    juce::ValueTree state("Parameters");
    
    // Essential state for the pattern
    state.setProperty("currentUPIInput", upiPattern, nullptr);
    state.setProperty("currentBackgroundColor", 0, nullptr); // Dark theme
    state.setProperty("bpm", 120.0, nullptr);
    state.setProperty("playing", false, nullptr);
    
    // Add UPI history with just this pattern
    state.setProperty("upiHistory", upiPattern, nullptr);
    
    return state;
}