/*
  ==============================================================================

    Rhythm Pattern Explorer - Plugin Editor
    User Interface for the AUv3 plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Plugin editor with simplified interface for mobile/desktop use
 * 
 * Features:
 * - Pattern type selection
 * - Onset/step count controls  
 * - BPM control
 * - Play/stop button
 * - Visual pattern display
 * - Pattern analysis display
 */
class RhythmPatternExplorerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                   public juce::Timer
{
public:
    RhythmPatternExplorerAudioProcessorEditor (RhythmPatternExplorerAudioProcessor&);
    ~RhythmPatternExplorerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for UI updates
    void timerCallback() override;
    
    // MIDI note access
    int getMidiNoteNumber() const;

private:
    //==============================================================================
    RhythmPatternExplorerAudioProcessor& audioProcessor;
    
    // UI Components
    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    
    juce::ComboBox patternTypeComboBox;
    juce::Label patternTypeLabel;
    
    juce::Slider onsetsSlider;
    juce::Label onsetsLabel;
    
    juce::Slider stepsSlider;
    juce::Label stepsLabel;
    
    juce::TextButton playButton;
    juce::TextButton generateButton;
    
    // UPI Pattern Input
    juce::TextEditor upiTextEditor;
    juce::Label upiLabel;
    juce::TextButton parseUPIButton;
    
    // Instance Name
    juce::TextEditor instanceNameEditor;
    juce::Label instanceNameLabel;
    
    // MIDI Note Number
    juce::Slider midiNoteSlider;
    juce::Label midiNoteLabel;
    
    juce::TextEditor patternDisplayEditor;
    juce::Label analysisLabel;
    juce::TextEditor versionEditor;
    
    // Attachments for automatic parameter binding
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> patternTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> onsetsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> playAttachment;
    
    // Visual pattern display
    void drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds);
    void updatePatternDisplay();
    void updateAnalysisDisplay();
    
    // Circle area for responsive layout
    juce::Rectangle<int> circleArea;
    
    // UPI handling
    void parseUPIPattern();
    void onParseButtonClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessorEditor)
};