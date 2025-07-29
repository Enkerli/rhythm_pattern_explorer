/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - Minimal AUv3 MIDI Effect
    Plugin Editor Header

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Minimal iPad AUv3 MIDI Effect Editor
 * 
 * Simple UI with a button to send MIDI notes for testing
 */
class RhythmPatternExplorerIPadAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RhythmPatternExplorerIPadAudioProcessorEditor (RhythmPatternExplorerIPadAudioProcessor&);
    ~RhythmPatternExplorerIPadAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RhythmPatternExplorerIPadAudioProcessor& audioProcessor;
    
    // UI Components
    juce::TextButton sendNoteButton;
    juce::Label titleLabel;
    juce::Label statusLabel;
    
    // Button callback
    void sendNoteButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerIPadAudioProcessorEditor)
};