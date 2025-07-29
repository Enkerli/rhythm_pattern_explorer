/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - Minimal AUv3 MIDI Effect
    Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RhythmPatternExplorerIPadAudioProcessorEditor::RhythmPatternExplorerIPadAudioProcessorEditor (RhythmPatternExplorerIPadAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up title label
    titleLabel.setText("Rhythm Pattern Explorer iPad", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Set up send note button
    sendNoteButton.setButtonText("Send MIDI Note (C4)");
    sendNoteButton.onClick = [this] { sendNoteButtonClicked(); };
    sendNoteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    sendNoteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(sendNoteButton);
    
    // Set up status label
    statusLabel.setText("Tap button to send MIDI note", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);

    // Set size suitable for iPad
    setSize (400, 300);
}

RhythmPatternExplorerIPadAudioProcessorEditor::~RhythmPatternExplorerIPadAudioProcessorEditor()
{
}

//==============================================================================
void RhythmPatternExplorerIPadAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background suitable for iPad
    g.fillAll (juce::Colours::darkgrey.darker());
    
    // Add a subtle border
    g.setColour (juce::Colours::lightgrey.withAlpha(0.3f));
    g.drawRect (getLocalBounds(), 2);
}

void RhythmPatternExplorerIPadAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);  // Add some padding
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(20);  // Spacing
    
    // Button in middle
    auto buttonBounds = bounds.removeFromTop(60);
    buttonBounds = buttonBounds.withSizeKeepingCentre(200, 50);
    sendNoteButton.setBounds(buttonBounds);
    
    bounds.removeFromTop(20);  // Spacing
    
    // Status at bottom
    statusLabel.setBounds(bounds.removeFromTop(30));
}

//==============================================================================
void RhythmPatternExplorerIPadAudioProcessorEditor::sendNoteButtonClicked()
{
    // Send a MIDI note (Middle C)
    audioProcessor.sendMidiNote(60, 100, 1);
    
    // Update status
    statusLabel.setText("MIDI Note Sent! (C4, Velocity 100)", juce::dontSendNotification);
    
    // Reset status after a short delay
    juce::Timer::callAfterDelay(1500, [this]() {
        statusLabel.setText("Tap button to send MIDI note", juce::dontSendNotification);
    });
}