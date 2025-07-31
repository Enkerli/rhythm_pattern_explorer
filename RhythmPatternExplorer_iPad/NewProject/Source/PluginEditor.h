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
    
    /// Core UI Components (desktop-compatible)
    juce::Label upiLabel;
    juce::TextEditor upiTextEditor;
    juce::TextButton tickButton;  // Equivalent to Parse/Tick button
    juce::Label patternDisplayEditor;  // Pattern text display
    
    /// Circle area for pattern visualization
    juce::Rectangle<int> circleArea;
    
    /// Background color cycling support
    enum class BackgroundColor {
        Dark = 0, White = 1, Green = 2, Orange = 3, Blue = 4, Purple = 5
    };
    BackgroundColor currentBackgroundColor = BackgroundColor::Dark;
    juce::Colour getBackgroundColour() const;
    
    /// Event handlers
    void parseUPIPattern();
    void onParseButtonClicked();
    void updatePatternDisplay();
    void updateStepSceneButton();
    
    /// Pattern visualization (desktop-compatible)
    void drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessorEditor)
};