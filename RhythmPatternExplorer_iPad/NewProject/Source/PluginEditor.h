/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - AUv3 Plugin
    Plugin Editor Header

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Audio Processor Editor for Rhythm Pattern Explorer iPad AUv3 plugin
 * 
 * Provides iPad-optimized UI for pattern input and visualization:
 * - Touch-friendly UPI pattern input
 * - Pattern step visualization
 * - Parameter controls optimized for iPad
 */
class RhythmPatternExplorerAudioProcessorEditor : public juce::AudioProcessorEditor, 
                                                   public juce::Timer
{
public:
    RhythmPatternExplorerAudioProcessorEditor (RhythmPatternExplorerAudioProcessor&);
    ~RhythmPatternExplorerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for updating debug display
    void timerCallback() override;

private:
    /// Reference to the audio processor
    RhythmPatternExplorerAudioProcessor& audioProcessor;
    
    /// UI Components for RPE
    juce::Label titleLabel;
    juce::TextEditor upiInputField;
    juce::TextButton triggerButton;
    juce::Label statusLabel;
    juce::Label patternDisplay;
    juce::TextEditor debugDisplay;
    
    /// Button callback methods
    void triggerButtonClicked();
    void upiInputChanged();
    void updatePatternDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessorEditor)
};