/*
  ==============================================================================

    iPad AUv3 MIDI Effect Template
    Plugin Editor Header

    A minimal working template for creating iPad AUv3 MIDI effect plugins
    using JUCE framework.
    
    This template is in the public domain (CC0 1.0).
    Feel free to use it as a starting point for your own plugins.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * @brief Template Audio Processor Editor for iPad AUv3 MIDI Effects
 * 
 * This class provides a minimal but complete UI foundation for iPad AUv3 
 * MIDI effect plugins. It demonstrates:
 * 
 * - Touch-optimized UI controls (44pt+ minimum touch targets)
 * - Responsive layout adapting to iPad orientations
 * - Real-time user feedback and status updates
 * - Thread-safe interaction with the audio processor
 * - Clean, modern visual design using JUCE's LookAndFeel
 * 
 * To customize this template:
 * 1. Rename this class to match your plugin
 * 2. Add your custom UI controls in the constructor
 * 3. Layout controls in the resized() method
 * 4. Handle user interactions with callbacks
 * 5. Update visual design in paint() method
 * 
 * The template includes a simple "Send MIDI Note" button to demonstrate
 * UI-to-processor communication.
 */
class TemplateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TemplateAudioProcessorEditor (TemplateAudioProcessor&);
    ~TemplateAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    /// Reference to the audio processor
    TemplateAudioProcessor& audioProcessor;
    
    /// UI Components (customize these for your plugin)
    juce::TextButton sendNoteButton;
    juce::Label titleLabel;
    juce::Label statusLabel;
    
    /// Button callback methods
    void sendNoteButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateAudioProcessorEditor)
};